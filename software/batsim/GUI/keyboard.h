#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "widget.h"
#include "display.h"

#define KEYBOARD_BG_COLOR			COLOR_BG_DEFAULT
#define KEYBOARD_BORDER_COLOR		COLOR_FG_DEFAULT
#define KEYBOARD_LINE_COLOR			COLOR_GRAY
#define KEYBOARD_SELECTED_COLOR		COLOR_SELECTED
#define KEYBOARD_SHIFT_ACTIVE_COLOR	COLOR_DARKGREEN

#define KEYBOARD_FONT				Font_Big
#define KEYBOARD_SPACING_X			31
#define KEYBOARD_SPACING_Y			30

typedef struct {
    widget_t base;
    void (*keyPressedCallback)(char);
    uint8_t selectedX, selectedY;
    uint8_t shift;
} keyboard_t;

keyboard_t* keyboard_new(void (*cb)(char));
void keyboard_draw(widget_t *w, coords_t offset);
void keyboard_input(widget_t *w, GUIEvent_t *ev);

#endif
