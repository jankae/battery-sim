#ifndef GUI_LABEL_H_
#define GUI_LABEL_H_

#include "widget.h"
#include "display.h"
#include "font.h"

#define LABEL_MAX_NAME      18

#define LABEL_FG_COLOR		COLOR_BLACK
#define LABEL_BG_COLOR		COLOR_BG_DEFAULT

typedef enum {LABEL_LEFT, LABEL_CENTER, LABEL_RIGHT} labelOrient_t;

typedef struct {
    widget_t base;
    char name[LABEL_MAX_NAME + 1];
    font_t font;
    labelOrient_t orient;
    uint32_t fontStartX;
} label_t;

label_t* label_newWithLength(const uint8_t length, const font_t font, const labelOrient_t orient);
label_t* label_newWithText(const char * const text, const font_t font);
void label_SetText(label_t *l, const char * const text);
void label_draw(widget_t *w, coords_t offset);
void label_input(widget_t *w, GUIEvent_t *ev);

#endif
