#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <stdio.h>
#include <time.h>

#define SECONDS_PER_DAY 24 * 60 * 60

#if defined __has_include && __has_include("voyah_pass_icons.h")
#include "voyah_pass_icons.h"
#else
extern const Icon I_logo_38x54;
#endif

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
} VoyahPassApp;
