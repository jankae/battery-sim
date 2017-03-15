#ifndef GUI_CONTAINER_H_
#define GUI_CONTAINER_H_

#include "widget.h"
#include "display.h"
#include "common.h"

#define CONTAINER_SCROLLBAR_SIZE    5

typedef struct {
    widget_t base;
    coords_t canvasSize;
    coords_t viewingSize;
    coords_t canvasOffset;
    coords_t scrollBarLength;
    struct {
        uint8_t editing :1;
        uint8_t focussed :1;
        uint8_t scrollVertical :1;
        uint8_t scrollHorizontal :1;
    } flags;
} container_t;

void container_create(container_t *c, int16_t width, int16_t height);
GUIResult_t container_attach(container_t *c, widget_t *w, int16_t x, int16_t y);
GUIResult_t container_draw(widget_t *w, coords_t offset);
void container_input(widget_t *w, GUIEvent_t *ev);
void container_focussed(widget_t *w);

#endif
