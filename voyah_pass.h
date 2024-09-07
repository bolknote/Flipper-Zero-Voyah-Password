#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <stdio.h>
#include <time.h>
#include <storage/storage.h>

#define VOYAH_PASS_TZ_FILE APP_DATA_PATH("tz.bin")

#if defined __has_include && __has_include("voyah_pass_icons.h")
#include "voyah_pass_icons.h"
#else
extern const Icon I_logo_38x54;
#endif

#define DESTRUCT(func) __attribute__((__cleanup__(func))) __auto_type

typedef struct {
    int8_t hours;
    uint8_t minutes;
} VoyahPassTZ;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    VoyahPassTZ* tz;
} VoyahPassApp;
