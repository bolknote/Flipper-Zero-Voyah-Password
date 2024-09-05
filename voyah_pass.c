#include "voyah_pass.h"
#include "voyah_pass_icons.h"

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

void voyah_pass_render_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);

    canvas_clear(canvas);

    const uint16_t y = (canvas_height(canvas) - icon_get_height(&I_logo_38x54)) / 2;
    canvas_draw_icon(canvas, 0, y, &I_logo_38x54);

    const size_t font_height = canvas_current_font_height(canvas);

    const uint16_t x = icon_get_width(&I_logo_38x54) + 5;
    canvas_draw_str(canvas, x, y + font_height, "Today's passwords:");

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

    FuriString* password = voyah_pass_get_pass(datetime.day, datetime.month, datetime.year);

    const uint32_t now = furi_hal_rtc_get_timestamp();

    DateTime tomorrow;
    datetime_timestamp_to_datetime(now + SECONDS_PER_DAY, &tomorrow);
    FuriString* tomorrow_password =
        voyah_pass_get_pass(tomorrow.day, tomorrow.month, tomorrow.year);

    DateTime yesterday;
    datetime_timestamp_to_datetime(now - SECONDS_PER_DAY, &yesterday);
    FuriString* yesterday_password =
        voyah_pass_get_pass(yesterday.day, yesterday.month, yesterday.year);

    furi_string_cat_printf(
        password,
        " or\n%s or\n%s",
        furi_string_get_cstr(tomorrow_password),
        furi_string_get_cstr(yesterday_password));

    elements_multiline_text(canvas, x, y + font_height * 2.5, furi_string_get_cstr(password));

    furi_string_free(tomorrow_password);
    furi_string_free(yesterday_password);
    furi_string_free(password);
}

static void voyah_pass_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
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

    return app;
}

void voyah_pass_app_free(VoyahPassApp** app) {
    furi_assert(*app);

    view_port_enabled_set((*app)->view_port, false);
    gui_remove_view_port((*app)->gui, (*app)->view_port);
    view_port_free((*app)->view_port);

    furi_record_close(RECORD_GUI);
    furi_message_queue_free((*app)->event_queue);

    free(*app);
}

int32_t voyah_pass_main(void* p) {
    UNUSED(p);

    __attribute__((__cleanup__(voyah_pass_app_free))) VoyahPassApp* app = voyah_pass_app_alloc();

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
