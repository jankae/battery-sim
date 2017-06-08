/*
 * window.h
 *
 *  Created on: Mar 15, 2017
 *      Author: jan
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"
#include "gui.h"

#define WINDOW_MAX_NAME         		16
#define WINDOW_BORDER_COLOR				COLOR_BLACK
#define WINDOW_TITLE_BG_COLOR			COLOR(65, 64, 59)
#define WINDOW_TITLE_FG_COLOR			COLOR(215, 214, 207)
#define WINDOW_CLOSE_AREA_COLOR			COLOR(229, 99, 42)
#define WINDOW_CLOSE_X_COLOR			COLOR_BG_DEFAULT

typedef struct {
    widget_t base;
    char title[WINDOW_MAX_NAME + 1];
    font_t font;
    widget_t *lastTopWidget;
    uint8_t lastPopup;
} window_t;

window_t* window_new(const char *titel, font_t font, coords_t size);
void window_destroy(window_t *w);
GUIResult_t window_SetMainWidget(window_t *w, widget_t *widg);
coords_t window_GetAvailableArea(window_t *w);
void window_draw(widget_t *w, coords_t offset);
void window_drawChildren(widget_t *w, coords_t offset);
void window_input(widget_t *w, GUIEvent_t *ev);

#endif /* WINDOW_H_ */
