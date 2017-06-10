#ifndef PROGRESSBAR_H_
#define PROGRESSBAR_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"

#define PROGRESSBAR_BORDER_COLOR		COLOR_BLACK
#define PROGRESSBAR_BG_COLOR			COLOR_BG_DEFAULT
#define PROGRESSBAR_BAR_COLOR			COLOR_GREEN

typedef struct {
    widget_t base;
    uint8_t state;
} progressbar_t;

progressbar_t* progressbar_new(coords_t size);
void progressbar_SetState(progressbar_t *p, uint8_t state);
void progressbar_draw(widget_t *w, coords_t offset);
void progressbar_input(widget_t *w, GUIEvent_t *ev);

#endif
