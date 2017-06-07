#ifndef ITEMCHOOSER_H_
#define ITEMCHOOSER_H_

#include "widget.h"
#include "display.h"

#define ITEMCHOOSER_SCROLLBAR_SIZE    	8
#define ITEMCHOOSER_SCROLLBAR_COLOR		COLOR_ORANGE

#define ITEMCHOOSER_BG_COLOR			COLOR_BG_DEFAULT
#define ITEMCHOOSER_BORDER_COLOR		COLOR_FG_DEFAULT
#define ITEMCHOOSER_SELECTED_BG_COLOR	COLOR(100, 100, 100)

typedef struct {
    widget_t base;
    uint8_t *value;
    const char * const * itemlist;
    font_t font;
    uint8_t lines;
    void (*changeCallback)(void);
    uint8_t topVisibleEntry;
} itemChooser_t;

itemChooser_t* itemChooser_new(const char * const * const items, uint8_t *value,
		font_t font, uint8_t visibleLines, uint8_t minSizeX);
void itemChooser_draw(widget_t *w, coords_t offset);
void itemChooser_input(widget_t *w, GUIEvent_t *ev);



#endif
