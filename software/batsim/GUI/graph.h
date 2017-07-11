#ifndef GRAPH_H_
#define GRAPH_H_

#include "widget.h"
#include "display.h"

#define GRAPH_BG_COLOR			COLOR_BG_DEFAULT
#define GRAPH_BORDER_COLOR		COLOR_FG_DEFAULT
#define GRAPH_TICKED_COLOR		COLOR(0, 192, 0)
#define GRAPH_UNTICKED_COLOR		COLOR(238, 0, 0)

typedef struct {
    widget_t base;
    int32_t *values;
    color_t color;
    unit_t *unit;
} graph_t;

graph_t* graph_new(int32_t *values, uint16_t num, uint16_t height, color_t color, unit_t *unit);
void graph_draw(widget_t *w, coords_t offset);
void graph_input(widget_t *w, GUIEvent_t *ev);
void graph_NewColor(graph_t *g, color_t color);
void graph_NewData(graph_t *g, int32_t *data);

#endif
