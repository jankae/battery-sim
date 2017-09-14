#include "button.h"

#include "buttons.h"

button_t* button_new(const char *name, font_t font, uint16_t minWidth, void (*cb)(widget_t*)) {
	button_t* button = (button_t*) pvPortMalloc(sizeof(button_t));
	if (!button) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) button);
    /* set widget functions */
    button->base.func.draw = button_draw;
    button->base.func.input = button_input;
    /* a button can't have any children */
    button->base.func.drawChildren = NULL;
    /* set name and callback */
    uint8_t i = 0;
    while (*name && i < BUTTON_MAX_NAME) {
        button->name[i++] = *name++;
    }
    button->name[i] = 0;
    button->callback = cb;
    button->font = font;
    button->pressed = 0;

    /* calculate size based on the font */
    button->base.size.y = font.height + 6;
    button->base.size.x = font.width * i + 5;

    if(minWidth > button->base.size.x)
        button->base.size.x = minWidth;

	/* calculate font start position */
	button->fontStart.y = 3;
	button->fontStart.x = (button->base.size.x - font.width * i - 1) / 2;

	return button;
}

void button_draw(widget_t *w, coords_t offset) {
    button_t *b = (button_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += b->base.size.x - 1;
    lowerRight.y += b->base.size.y - 1;
    /* draw outline */
	if (w->flags.selected) {
		display_SetForeground(COLOR_SELECTED);
	} else if(w->flags.selectable){
		display_SetForeground(BUTTON_FG_COLOR);
	} else {
		display_SetForeground(COLOR_GRAY);
	}
    display_VerticalLine(upperLeft.x, upperLeft.y + 1, b->base.size.y - 2);
    display_VerticalLine(lowerRight.x, upperLeft.y + 1, b->base.size.y - 2);
    display_HorizontalLine(upperLeft.x + 1, upperLeft.y, b->base.size.x - 2);
    display_HorizontalLine(upperLeft.x + 1, lowerRight.y, b->base.size.x - 2);
	if (w->flags.selectable) {
		if (!b->pressed)
			display_SetForeground(
					color_Tint(BUTTON_FG_COLOR, BUTTON_BG_COLOR, 75));
		else
			display_SetForeground(
					color_Tint(BUTTON_FG_COLOR, COLOR_WHITE, 200));
	}
    display_VerticalLine(lowerRight.x - 1, upperLeft.y + 1, b->base.size.y - 2);
    display_HorizontalLine(upperLeft.x + 1, lowerRight.y - 1,
            b->base.size.x - 2);

//    display_SetForeground(color_Tint(fg, bg, 150));
//    display_VerticalLine(lowerRight.x - 2, upperLeft.y + 1, b->base.size.y - 3);
//    display_HorizontalLine(upperLeft.x + 1, lowerRight.y - 2,
//            b->base.size.x - 3);

	if (w->flags.selectable) {
		if (b->pressed)
			display_SetForeground(
					color_Tint(BUTTON_FG_COLOR, BUTTON_BG_COLOR, 75));
		else
			display_SetForeground(
					color_Tint(BUTTON_FG_COLOR, COLOR_WHITE, 200));
	}
    display_VerticalLine(upperLeft.x + 1, upperLeft.y + 1, b->base.size.y - 3);
    display_HorizontalLine(upperLeft.x + 1, upperLeft.y + 1,
            b->base.size.x - 3);

    if (b->name) {
    	if (b->base.flags.selectable) {
    		display_SetForeground(BUTTON_FG_COLOR);
    	} else {
    		display_SetForeground(COLOR_GRAY);
    	}
    	display_SetBackground(BUTTON_BG_COLOR);
		display_SetFont(b->font);
		display_String(upperLeft.x + b->fontStart.x,
				upperLeft.y + b->fontStart.y, b->name);
	}
//    if (b->base.flags.selected) {
//        /* invert button area */
//        screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1,
//                lowerRight.x - 2, lowerRight.y - 2, PIXEL_INVERT);
//    }
    return;
}

void button_input(widget_t *w, GUIEvent_t *ev) {
    button_t *b = (button_t*) w;
    switch(ev->type) {
    case EVENT_TOUCH_PRESSED:
		if (!b->pressed) {
			b->pressed = 1;
			widget_RequestRedraw(w);
		}
		break;
    case EVENT_TOUCH_RELEASED:
		if (b->pressed) {
			b->pressed = 0;
			widget_RequestRedraw(w);
			if (b->callback)
				b->callback(w);
		}
		break;
    case EVENT_BUTTON_CLICKED:
    	if(ev->button & (BUTTON_UNIT1|BUTTON_ENCODER)) {
			if (b->callback)
				b->callback(w);
    	}
		break;
    default:
    	break;
	}
    return;
}