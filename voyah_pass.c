#include "voyah_pass.h"

FuriString* voyah_pass_get_pass(uint16_t day, uint16_t month, uint16_t year) {
    FuriString* password = furi_string_alloc();

    uint16_t dm = day + month * 100;

    for(size_t i = 0; i < 4; i++) {
        FuriString* v = furi_string_alloc_printf(
            "%u%s", (dm % 10) + (year % 10), furi_string_get_cstr(password));
        dm /= 10;
        year /= 10;

        furi_string_set(password, v);
        furi_string_free(v);
    }

    return password;
}

bool voyah_pass_read_tz(VoyahPassTZ* tz) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(!storage_file_open(file, VOYAH_PASS_TZ_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        return false;
    }

    bool result = storage_file_read(file, tz, sizeof(VoyahPassTZ)) == sizeof(VoyahPassTZ);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

void voyah_pass_set_tz_button(Canvas* canvas, const VoyahPassTZ* tz) {
    furi_check(canvas);

    if(tz != NULL) {
        FuriString* tz_str = furi_string_alloc_printf("%+0d:%0d", tz->hours, tz->minutes);
        elements_button_right(canvas, furi_string_get_cstr(tz_str));
        furi_string_free(tz_str);
    } else {
        elements_button_right(canvas, "Set TZ");
    }
}

void voyah_pass_print_password(Canvas* canvas, int32_t x, int32_t y, const VoyahPassTZ* tz) {
    furi_check(canvas);

    const uint32_t now = furi_hal_rtc_get_timestamp();
    FuriString* password;
    DateTime datetime;

    if(tz == NULL) {
        furi_hal_rtc_get_datetime(&datetime);
        password = voyah_pass_get_pass(datetime.day, datetime.month, datetime.year);

        DateTime tomorrow;
        datetime_timestamp_to_datetime(now + 24 * 60 * 60, &tomorrow);
        FuriString* tomorrow_password =
            voyah_pass_get_pass(tomorrow.day, tomorrow.month, tomorrow.year);

        DateTime yesterday;
        datetime_timestamp_to_datetime(now - 24 * 60 * 60, &yesterday);
        FuriString* yesterday_password =
            voyah_pass_get_pass(yesterday.day, yesterday.month, yesterday.year);

        furi_string_cat_printf(
            password,
            " or\n%s or\n%s",
            furi_string_get_cstr(tomorrow_password),
            furi_string_get_cstr(yesterday_password));

        furi_string_free(tomorrow_password);
        furi_string_free(yesterday_password);
    } else {
        const uint32_t GMT = now - (tz->hours * 60 + tz->minutes) * 60;
        const uint32_t china_tz = GMT + 8 * 60 * 60;

        datetime_timestamp_to_datetime(china_tz, &datetime);
        password = voyah_pass_get_pass(datetime.day, datetime.month, datetime.year);
    }

    elements_multiline_text(canvas, x, y, furi_string_get_cstr(password));

    furi_string_free(password);
}

void voyah_pass_render_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    const VoyahPassTZ* tz = ((VoyahPassApp*)ctx)->tz;

    const uint16_t y = (canvas_height(canvas) - icon_get_height(&I_logo_38x54)) / 2;
    canvas_draw_icon(canvas, 0, y, &I_logo_38x54);

    const size_t font_height = canvas_current_font_height(canvas);

    const uint16_t x = icon_get_width(&I_logo_38x54) + 5;

    voyah_pass_set_tz_button(canvas, tz);
    canvas_draw_str(
        canvas, x, y + font_height, tz != NULL ? "Today's password:" : "Today's passwords:");
    voyah_pass_print_password(canvas, x, y + font_height * 2.5, tz);
}

static void voyah_pass_input_callback(InputEvent* input_event, void* ctx) {
    furi_message_queue_put((FuriMessageQueue*)ctx, input_event, FuriWaitForever);
}

VoyahPassApp* voyah_pass_app_alloc() {
    VoyahPassApp* app = malloc(sizeof(VoyahPassApp));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, voyah_pass_render_callback, app);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_input_callback_set(app->view_port, voyah_pass_input_callback, app->event_queue);

    app->tz = malloc(sizeof(VoyahPassTZ));
    if(!voyah_pass_read_tz(app->tz)) {
        free(app->tz);
        app->tz = NULL;
    }

    return app;
}

void voyah_pass_app_free(VoyahPassApp** app) {
    view_port_enabled_set((*app)->view_port, false);
    gui_remove_view_port((*app)->gui, (*app)->view_port);
    view_port_free((*app)->view_port);

    furi_record_close(RECORD_GUI);
    furi_message_queue_free((*app)->event_queue);
    free((*app)->tz);

    free(*app);
}

int32_t voyah_pass_main(void* p) {
    UNUSED(p);
    DESTRUCT(voyah_pass_app_free) app = voyah_pass_app_alloc();

    for(InputEvent event;;) {
        furi_check(
            furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk);

        if(event.type == InputTypeShort) {
            switch(event.key) {
            case InputKeyBack:
                return 0;

            default:
                break;
            }

            view_port_update(app->view_port);
        }
    }
}
