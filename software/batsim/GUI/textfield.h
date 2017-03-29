#ifndef GUI_TEXTFIELD_H_
#define GUI_TEXTFIELD_H_

#include "widget.h"
#include "display.h"
#include "font.h"

#define TEXTFIELD_FG_COLOR		COLOR_BLACK
#define TEXTFIELD_BG_COLOR		COLOR_BG_DEFAULT

typedef struct {
    widget_t base;
    char *text;
    font_t font;
} textfield_t;

textfield_t* textfield_new(const char *text, const font_t font,
		coords_t maxSize);
void textfield_draw(widget_t *w, coords_t offset);
void textfield_input(widget_t *w, GUIEvent_t *ev);

#endif
