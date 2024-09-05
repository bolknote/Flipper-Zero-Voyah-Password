#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <stdio.h>
#include <time.h>

#define SECONDS_PER_DAY 24 * 60 * 60

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
} VoyahPassApp;
