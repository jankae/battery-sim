#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"

#define BUTTON_MAX_NAME         16

#define BUTTON_FG_COLOR			COLOR_BLACK
#define BUTTON_BG_COLOR			COLOR_BG_DEFAULT

typedef struct {
    widget_t base;
    void (*callback)(void);
    char name[BUTTON_MAX_NAME + 1];
    font_t font;
    coords_t fontStart;
    uint8_t pressed;
} button_t;

button_t* button_new(const char *name, font_t font, uint8_t minWidth, void *cb);
GUIResult_t button_draw(widget_t *w, coords_t offset);
void button_input(widget_t *w, GUIEvent_t *ev);

#endif
