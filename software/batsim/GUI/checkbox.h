#ifndef GUI_CHECKBOX_H_
#define GUI_CHECKBOX_H_

#include "widget.h"
#include "display.h"

#define CHECKBOX_BG_COLOR			COLOR_BG_DEFAULT
#define CHECKBOX_BORDER_COLOR		COLOR_FG_DEFAULT
#define CHECKBOX_TICKED_COLOR		COLOR(0, 192, 0)
#define CHECKBOX_UNTICKED_COLOR		COLOR(238, 0, 0)

typedef struct {
    widget_t base;
    void (*callback)(widget_t* source);
    uint8_t *value;
    coords_t fontStart;
} checkbox_t;

checkbox_t* checkbox_new(uint8_t *value, void (*cb)(widget_t*));
void checkbox_draw(widget_t *w, coords_t offset);
void checkbox_input(widget_t *w, GUIEvent_t *ev);

#endif
