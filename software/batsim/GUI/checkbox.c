#include "checkbox.h"

#include "buttons.h"

checkbox_t* checkbox_new(uint8_t *value, void (*cb)(widget_t*)) {
	checkbox_t* c = pvPortMalloc(sizeof(checkbox_t));
	if (!c) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) c);
    /* set widget functions */
    c->base.func.draw = checkbox_draw;
    c->base.func.input = checkbox_input;
    /* a textfield can't have any children */
    c->base.func.drawChildren = NULL;
    /* set callback */
    c->value = value;
    c->callback = cb;
    c->base.size.x = 29;
    c->base.size.y = 29;

    return c;
}

void checkbox_draw(widget_t *w, coords_t offset) {
    checkbox_t *c = (checkbox_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += c->base.size.x - 1;
    lowerRight.y += c->base.size.y - 1;
	if (w->flags.selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground(CHECKBOX_BORDER_COLOR);
	}
    display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
    if (*c->value) {
    	display_SetForeground(CHECKBOX_TICKED_COLOR);
		display_Line(upperLeft.x + 2, lowerRight.y - w->size.y / 3,
				upperLeft.x + w->size.x / 3, lowerRight.y - 2);
		display_Line(upperLeft.x + 2, lowerRight.y - w->size.y / 3 - 1,
				upperLeft.x + w->size.x / 3 + 1, lowerRight.y - 2);
		display_Line(upperLeft.x + 2, lowerRight.y - w->size.y / 3 - 2,
				upperLeft.x + w->size.x / 3 + 2, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 2,
				upperLeft.x + w->size.x / 3, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 3,
				upperLeft.x + w->size.x / 3 + 1, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 4,
				upperLeft.x + w->size.x / 3 + 2, lowerRight.y - 2);
    } else {
    	display_SetForeground(CHECKBOX_UNTICKED_COLOR);
    	display_Line(upperLeft.x + 3, upperLeft.y + 3, lowerRight.x - 3,
                lowerRight.y - 3);
    	display_Line(upperLeft.x + 4, upperLeft.y + 3, lowerRight.x - 3,
                lowerRight.y - 4);
    	display_Line(upperLeft.x + 3, upperLeft.y + 4, lowerRight.x - 4,
                lowerRight.y - 3);
    	display_Line(upperLeft.x + 3, lowerRight.y - 3, lowerRight.x - 3,
                upperLeft.y + 3);
    	display_Line(upperLeft.x + 4, lowerRight.y - 3, lowerRight.x - 3,
                upperLeft.y + 4);
    	display_Line(upperLeft.x + 3, lowerRight.y - 4, lowerRight.x - 4,
                upperLeft.y + 3);
    }
}

void checkbox_input(widget_t *w, GUIEvent_t *ev) {
	checkbox_t *c = (checkbox_t*) w;
	switch(ev->type) {
	case EVENT_BUTTON_CLICKED:
		if(ev->button!=BUTTON_ENTER && ev->button != BUTTON_ENCODER) {
			break;
		}
		/* no break */
	case EVENT_TOUCH_RELEASED:
		*c->value = !*c->value;
		widget_RequestRedrawFull(w);
		if (c->callback)
			c->callback(w);
		ev->type = EVENT_NONE;
		break;
	default:
		break;
	}
	return;
}
