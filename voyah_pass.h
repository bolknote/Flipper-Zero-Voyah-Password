#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <stdio.h>
#include <storage/storage.h>

#define VOYAH_PASS_TZ_FILE APP_DATA_PATH("tz.bin")
#define VOYAH_PASS_MAX_TZ  14
#define VOYAH_PASS_MIN_TZ  -12

#if __has_include("voyah_pass_icons.h")
#include "voyah_pass_icons.h"
#else
extern const Icon I_logo_38x54;
extern const Icon I_right_8x15;
extern const Icon I_left_8x15;
#endif

#define DESTRUCT(func) __attribute__((__cleanup__(func))) __auto_type

typedef enum {
    InitialState = 0,
    DialogState = 1,
} VoyahPassState;

typedef struct {
    int8_t hours;
    uint8_t minutes;
} VoyahPassTZ;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    VoyahPassTZ *tz, *tz_backup;
    VoyahPassState state;
    uint8_t sel_pos;
} VoyahPassApp;
