#include "itemChooser.h"

itemChooser_t* itemChooser_new(const char * const * const items, uint8_t *value,
		font_t font, uint8_t visibleLines, uint8_t minSizeX) {
	itemChooser_t* i = pvPortMalloc(sizeof(itemChooser_t));
	if (!i) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) i);
    /* set widget functions */
    i->base.func.draw = itemChooser_draw;
    i->base.func.input = itemChooser_input;
    /* a textfield can't have any children */
    i->base.func.drawChildren = NULL;

    /* set member variables */
    i->changeCallback = NULL;
    i->font = font;
    i->itemlist = items;
    i->value = value;
    i->lines = visibleLines;
    i->topVisibleEntry = 0;
    /* find number of items and longest item */
    uint8_t maxLength = 0;
    uint8_t numItems;
    for (numItems = 0; i->itemlist[numItems]; numItems++) {
        uint8_t length = strlen(i->itemlist[numItems]);
        if (length > maxLength)
            maxLength = length;
    }
    /* calculate size */
    i->base.size.y = font.height * visibleLines + 3;
    i->base.size.x = font.width * maxLength + 3 + ITEMCHOOSER_SCROLLBAR_SIZE;
    if (i->base.size.x < minSizeX) {
        i->base.size.x = minSizeX;
    }
    return i;
}

void itemChooser_draw(widget_t *w, coords_t offset) {
	itemChooser_t *i = (itemChooser_t*) w;

	/* Get number of items */
    uint8_t numItems;
	for (numItems = 0; i->itemlist[numItems]; numItems++)
		;
    /* Constrain selected item (just in case) */
    if (*i->value >= numItems) {
        *i->value = numItems - 1;
    }

    /* Update visible entry offset */
    if(*i->value < i->topVisibleEntry) {
    	i->topVisibleEntry = *i->value;
    } else if(*i->value >= i->topVisibleEntry + i->lines) {
    	i->topVisibleEntry = *i->value - i->lines + 1;
    }

    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += i->base.size.x - 1;
    lowerRight.y += i->base.size.y - 1;

    /* Draw surrounding rectangle */
	if (w->flags.selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground(ITEMCHOOSER_BORDER_COLOR);
	}
    display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
	display_SetForeground(ITEMCHOOSER_BORDER_COLOR);

    /* Display items */
    uint8_t line;
    for(line = 0;line < i->lines; line++) {
    	uint8_t index = line + i->topVisibleEntry;
    	if(index == *i->value) {
    		/* this is the currently selected entry */
    		display_SetBackground(ITEMCHOOSER_SELECTED_BG_COLOR);
    	} else {
    		display_SetBackground(COLOR_BG_DEFAULT);
    	}
		display_String(upperLeft.x + 1, upperLeft.y + 2 + line * i->font.height,
				i->itemlist[index]);
		/* fill rectangle between text and and scrollbar begin with background color */
		uint16_t xbegin = upperLeft.x + 1
				+ i->font.width * strlen(i->itemlist[index]);
		uint16_t ybegin = upperLeft.y + 2 + line * i->font.height;
		uint16_t xstop = lowerRight.x - ITEMCHOOSER_SCROLLBAR_SIZE - 1;
		uint16_t ystop = ybegin + i->font.height - 1;
		display_SetForeground(display_GetBackground());
		display_RectangleFull(xbegin, ybegin, xstop, ystop);
		display_SetForeground(ITEMCHOOSER_BORDER_COLOR);
    }
    /* display scrollbar */
    display_SetForeground(ITEMCHOOSER_BORDER_COLOR);
	display_VerticalLine(lowerRight.x - ITEMCHOOSER_SCROLLBAR_SIZE,
			upperLeft.y, i->base.size.y);
	/* calculate beginning and end of scrollbar */
	uint8_t scrollBegin = common_Map(i->topVisibleEntry, 0, numItems, 0,
			i->base.size.y);
	uint8_t scrollEnd = common_Map(i->topVisibleEntry + i->lines, 0, numItems,
			0, i->base.size.y);
	/* display position indicator */
	display_SetForeground(ITEMCHOOSER_SCROLLBAR_COLOR);
	display_RectangleFull(lowerRight.x - ITEMCHOOSER_SCROLLBAR_SIZE + 1,
			upperLeft.y + scrollBegin + 1, lowerRight.x - 1,
			upperLeft.y + scrollEnd - 2);
}

void itemChooser_input(widget_t *w, GUIEvent_t *ev) {
	itemChooser_t *i = (itemChooser_t*) w;

	/* Get number of items */
    uint8_t numItems;
	for (numItems = 0; i->itemlist[numItems]; numItems++)
		;
    /* Constrain selected item (just in case) */
    if (*i->value >= numItems) {
        *i->value = numItems - 1;
    }

	switch(ev->type) {
	case EVENT_ENCODER_MOVED: {
		int16_t newVal = *i->value + ev->movement;
		if (newVal < 0) {
			newVal = 0;
		} else if (newVal >= numItems) {
			newVal = numItems - 1;
		}
		if (*i->value != newVal) {
			*i->value = newVal;
			widget_RequestRedrawFull(w);
		}
	}
		break;
	case EVENT_TOUCH_PRESSED:
		if(w->flags.selected) {
			/* only react to touch if already selected. This allows
			 * the user to select the widget without already changing the value */
			int16_t newVal = i->topVisibleEntry + (ev->pos.y - 2) / i->font.height;
			if (newVal < 0) {
				newVal = 0;
			} else if (newVal >= numItems) {
				newVal = numItems - 1;
			}
			if (*i->value != newVal) {
				*i->value = newVal;
				widget_RequestRedrawFull(w);
			}
		}
		break;
	default:
		break;
	}
	return;
}
