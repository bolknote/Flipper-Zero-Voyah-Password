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
    const size_t tz_size = sizeof(VoyahPassTZ);

    bool result = storage_file_open(file, VOYAH_PASS_TZ_FILE, FSAM_READ, FSOM_OPEN_EXISTING);
    result = result && storage_file_read(file, tz, tz_size) == tz_size;

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

bool voyah_pass_write_tz(VoyahPassTZ* tz) {
    furi_check(tz);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    const size_t tz_size = sizeof(VoyahPassTZ);

    bool result = storage_file_open(file, VOYAH_PASS_TZ_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    result = result && storage_file_write(file, tz, tz_size) == tz_size;

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

void voyah_pass_set_tz_button(Canvas* canvas, const VoyahPassTZ* tz) {
    furi_check(canvas);

    if(tz == NULL) {
        elements_button_right(canvas, "Set TZ");
    } else {
        FuriString* tz_str = furi_string_alloc_printf("%+03d:%02d", tz->hours, tz->minutes);
        elements_button_right(canvas, furi_string_get_cstr(tz_str));
        furi_string_free(tz_str);
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

void voyah_pass_draw_dialog(Canvas* canvas, const VoyahPassTZ* tz, VoyahPassApp* app) {
    const int8_t hours = tz == NULL ? 0 : tz->hours;
    const uint8_t minutes = tz == NULL ? 0 : tz->minutes;

    elements_text_box(
        canvas,
        0,
        0,
        canvas_width(canvas),
        canvas_height(canvas),
        AlignCenter,
        AlignTop,
        "Set your timezone",
        false);

    FuriString* str_hours = furi_string_alloc_printf("%+03d", hours);
    FuriString* str_minutes = furi_string_alloc_printf("%02d", minutes);

    canvas_set_font(canvas, FontBigNumbers);
    canvas_set_color(canvas, ColorXOR);

    FuriString* str = furi_string_alloc_set(str_hours);
    furi_string_cat_str(str, ":");
    furi_string_cat(str, str_minutes);

    const size_t w_str = canvas_string_width(canvas, furi_string_get_cstr(str));
    const size_t font_h = canvas_current_font_height(canvas);
    const int32_t x = (canvas_width(canvas) - w_str) / 2;
    const int32_t y = canvas_height(canvas) / 2 + 4;
    const uint16_t padding = 3;
    uint16_t h_width;
    int32_t x_box;

    if(app->sel_pos == 1) {
        x_box = x + canvas_string_width(canvas, furi_string_get_cstr(str_hours)) +
                canvas_glyph_width(canvas, ':') + 1;
        h_width = canvas_string_width(canvas, furi_string_get_cstr(str_minutes));
    } else {
        h_width = canvas_string_width(canvas, furi_string_get_cstr(str_hours));
        x_box = x;
    }

    canvas_draw_box(
        canvas, x_box - padding, y - font_h, h_width + padding * 2, font_h + padding * 2);

    canvas_draw_str(canvas, x, y, furi_string_get_cstr(str));

    furi_string_free(str_minutes);
    furi_string_free(str_hours);
    furi_string_free(str);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    const int32_t icon_padding = 10;
    const int32_t x_right = x + w_str + icon_padding;
    const int32_t x_left = x - icon_get_width(&I_left_8x15) - icon_padding;

    canvas_draw_icon(canvas, x_right, y - icon_get_height(&I_right_8x15) + 1, &I_right_8x15);
    canvas_draw_icon(canvas, x_left, y - icon_get_height(&I_left_8x15) + 1, &I_left_8x15);

    elements_button_center(canvas, "Set");
}

void voyah_pass_init_tz(VoyahPassApp* app) {
    if(app->tz == NULL) {
        app->tz = malloc(sizeof(VoyahPassTZ));
    }
}

void voyah_pass_up_pressed(VoyahPassApp* app) {
    furi_check(app->tz);

    switch(app->sel_pos) {
    case 0:
        if(++app->tz->hours > VOYAH_PASS_MAX_TZ) app->tz->hours = VOYAH_PASS_MAX_TZ;
        break;

    default:
        app->tz->minutes = app->tz->minutes == 60 - 15 ? 00 : app->tz->minutes + 15;
        break;
    }
}

void voyah_pass_down_pressed(VoyahPassApp* app) {
    furi_check(app->tz);

    switch(app->sel_pos) {
    case 0:
        if(--app->tz->hours < VOYAH_PASS_MIN_TZ) app->tz->hours = VOYAH_PASS_MIN_TZ;
        break;

    default:
        app->tz->minutes = app->tz->minutes == 0 ? 60 - 15 : app->tz->minutes - 15;
        break;
    }
}

void voyah_pass_render_callback(Canvas* canvas, void* ctx) {
    furi_check(ctx);
    canvas_set_color(canvas, ColorBlack);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    const VoyahPassTZ* tz = ((VoyahPassApp*)ctx)->tz;

    if(((VoyahPassApp*)ctx)->state == DialogState) {
        voyah_pass_draw_dialog(canvas, tz, (VoyahPassApp*)ctx);
    } else {
        const uint16_t y = (canvas_height(canvas) - icon_get_height(&I_logo_38x54)) / 2;
        canvas_draw_icon(canvas, 0, y, &I_logo_38x54);

        const size_t font_height = canvas_current_font_height(canvas);

        const uint16_t x = icon_get_width(&I_logo_38x54) + 5;

        canvas_draw_str(
            canvas, x, y + font_height, tz == NULL ? "Today's passwords:" : "Today's password:");
        voyah_pass_print_password(canvas, x, y + font_height * 2.5, tz);

        voyah_pass_set_tz_button(canvas, tz);
    }
}

static void voyah_pass_input_callback(InputEvent* input_event, void* ctx) {
    furi_message_queue_put((FuriMessageQueue*)ctx, input_event, FuriWaitForever);
}

VoyahPassApp* voyah_pass_app_alloc() {
    VoyahPassApp* app = malloc(sizeof(VoyahPassApp));

    app->state = InitialState;
    app->sel_pos = 0;

    voyah_pass_init_tz(app);
    if(!voyah_pass_read_tz(app->tz)) {
        free(app->tz);
        app->tz = NULL;
    }

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, voyah_pass_render_callback, app);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_input_callback_set(app->view_port, voyah_pass_input_callback, app->event_queue);

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
            if(app->state == DialogState) {
                switch(event.key) {
                case InputKeyRight:
                case InputKeyLeft:
                    app->sel_pos = 1 - app->sel_pos;
                    break;

                case InputKeyBack:
                    app->state = InitialState;
                    if(app->tz_backup == NULL) {
                        free(app->tz);
                        app->tz = NULL;
                    } else {
                        memcpy(app->tz, app->tz_backup, sizeof(VoyahPassTZ));
                        free(app->tz_backup);
                    }
                    break;

                case InputKeyUp:
                    voyah_pass_up_pressed(app);
                    break;

                case InputKeyDown:
                    voyah_pass_down_pressed(app);
                    break;

                case InputKeyOk:
                    app->state = InitialState;
                    voyah_pass_write_tz(app->tz);
                    break;

                default:
                    break;
                }
            } else {
                switch(event.key) {
                case InputKeyBack:
                    return 0;
                    break;

                case InputKeyRight:
                    app->state = DialogState;
                    if(app->tz == NULL) {
                        app->tz_backup = NULL;
                    } else {
                        app->tz_backup = malloc(sizeof(VoyahPassTZ));
                        memcpy(app->tz_backup, app->tz, sizeof(VoyahPassTZ));
                    }
                    voyah_pass_init_tz(app);
                    break;

                default:
                    break;
                }
            }
            view_port_update(app->view_port);
        }
    }
}
