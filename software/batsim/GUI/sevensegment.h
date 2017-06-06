#ifndef GUI_SEVEN_H_
#define GUI_SEVEN_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"
#include "dialog.h"

#define SEVENSEGMENT_BG_COLOR				COLOR_BG_DEFAULT
#define SEVENSEGMENT_FG_COLOR				COLOR_FG_DEFAULT
#define SEVENSEGMENT_BORDER_COLOR			COLOR_FG_DEFAULT

typedef struct {
    widget_t base;
    int32_t *value;
    uint8_t segmentLength;
    uint8_t segmentWidth;
    uint8_t length;
    uint8_t dot;
    color_t color;
} sevensegment_t;

sevensegment_t* sevensegment_new(int32_t *value, uint8_t sLength, uint8_t sWidth, uint8_t length, uint8_t dot, color_t color);

void sevensegment_draw(widget_t *w, coords_t offset);
void sevensegment_input(widget_t *w, GUIEvent_t *ev);

#endif
