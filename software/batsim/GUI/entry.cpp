#include "entry.h"

#include "buttons.h"

entry_t* entry_new(int32_t *value, const int32_t *max, const int32_t *min,
		font_t font, uint8_t length, const unit_t * const unit) {
	entry_t* e = (entry_t*) pvPortMalloc(sizeof(entry_t));
	if (!e) {
		/* malloc failed */
		return NULL;
	}
	widget_init((widget_t*) e);
	/* set widget functions */
	e->base.func.draw = entry_draw;
	e->base.func.input = entry_input;
	/* set member variables */
	e->value = value;
	e->max = max;
	e->min = min;
	e->font = font;
	e->unit = unit;
	e->length = length;
    e->changeCallback = NULL;
    e->flags.editing = 0;
	e->base.size.y = font.height + 3;
	e->base.size.x = font.width * length + 3;

	return e;
}

uint32_t entry_GetInputStringValue(entry_t *e, uint32_t multiplier) {
    uint32_t value = 0;
    uint8_t i;
    uint32_t div = 0;
    for (i = 0; i < e->length; i++) {
        if (e->inputString[i] >= '0' && e->inputString[i] <= '9') {
            value *= 10;
            value += e->inputString[i] - '0';
            if (div) {
                div *= 10;
            }
        } else if (e->inputString[i] == '.') {
            div = 1;
        }
    }
    value *= multiplier;
    if (div)
        value /= div;
    return value;
}
//
//uint32_t entry_getIncrement(entry_t *e) {
//    /* get dot position from unit prefix */
//    uint32_t dotMult = 1;
//    if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][1])) {
//        dotMult = 1000;
//    } else if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][2])) {
//        dotMult = 1000000;
//    }
//    /* find dot position */
//    uint8_t dot;
//    for (dot = 0; dot <= e->digits; dot++) {
//        if (e->inputString[dot] == '.')
//            break;
//    }
//    while (e->encEditPos < dot - 1) {
//        dotMult *= 10;
//        dot--;
//    }
//    while (e->encEditPos > dot) {
//        dotMult /= 10;
//        dot++;
//    }
//    return dotMult;
//}
//
//void entry_setIncrement(entry_t *e, uint32_t increment) {
//    /* get dot position from unit prefix */
//    uint32_t dotMult = 1;
//    if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][1])) {
//        dotMult = 1000;
//    } else if (!strcmp(&e->inputString[e->digits + 1], unitNames[e->unit][2])) {
//        dotMult = 1000000;
//    }
//    /* find dot position */
//    uint8_t dot;
//    for (dot = 0; dot <= e->digits; dot++) {
//        if (e->inputString[dot] == '.')
//            break;
//    }
//    e->encEditPos = dot - 1;
//    if (increment > dotMult) {
//        while (increment > dotMult) {
//            if (!e->encEditPos) {
//                /* reached end of setting */
//                return;
//            }
//            e->encEditPos--;
//            dotMult *= 10;
//        }
//    } else if (increment < dotMult) {
//        e->encEditPos++;
//        while (increment < dotMult) {
//            e->encEditPos++;
//            dotMult /= 10;
//        }
//    }
//    if (e->encEditPos > e->digits) {
//        e->encEditPos = e->digits;
//    }
//}

int32_t entry_constrainValue(entry_t *e, int32_t value) {
    if (e->max && value > *e->max) {
        return *e->max;
    } else if (e->min && value < *e->min) {
        return *e->min;
    } else {
        return value;
    }
}

void entry_draw(widget_t *w, coords_t offset) {
    entry_t *e = (entry_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
	coords_t lowerRight = upperLeft;
	lowerRight.x += e->base.size.x - 1;
	lowerRight.y += e->base.size.y - 1;
	if (w->flags.selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground(ENTRY_BORDER_COLOR);
	}
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

	/* display string */
	if (!e->flags.editing) {
		/* construct value string */
		common_StringFromValue(e->inputString, e->length, *e->value, e->unit);
		if (w->flags.selectable) {
			display_SetForeground(ENTRY_FG_COLOR);
		} else {
			display_SetForeground(COLOR_GRAY);
		}
	} else {
		display_SetForeground(COLOR_SELECTED);
	}
	if (e->base.flags.selectable) {
		display_SetBackground(ENTRY_BG_COLOR);
	} else {
		display_SetBackground(COLOR_UNSELECTABLE);
	}
	display_SetFont(e->font);
	display_String(upperLeft.x + 1, upperLeft.y + 2, e->inputString);
}

void entry_input(widget_t *w, GUIEvent_t *ev) {
    entry_t *e = (entry_t*) w;
    switch(ev->type) {
    case EVENT_BUTTON_CLICKED:
		if (BUTTON_IS_INPUT(ev->button)) {
			widget_RequestRedraw(w);
			if (!e->flags.editing) {
				/* Start editing */
				e->flags.editing = 1;
				e->editPos = 0;
				e->flags.dotSet = 0;
				memset(e->inputString, ' ', e->length);
				e->inputString[e->length] = 0;
			}
			/* Add button input to inputString */
			if (ev->button == BUTTON_DOT) {
				if (e->editPos < e->length && !e->flags.dotSet) {
					/* add dot */
					e->inputString[e->editPos++] = '.';
					e->flags.dotSet = 1;
				}
			} else if (ev->button == BUTTON_SIGN) {
				/* toggle sign */
				if(e->inputString[0] == '-') {
					/* remove sign */
					memmove(e->inputString, &e->inputString[1], e->editPos - 1);
					e->editPos--;
					e->inputString[e->editPos] = ' ';
				} else if(e->editPos < e->length) {
					/* add sign */
					memmove(&e->inputString[1], &e->inputString[0], e->editPos);
					e->inputString[0] = '-';
					e->editPos++;
				}
			} else {
				/* must be a number input */
				if(e->editPos < e->length) {
					e->inputString[e->editPos++] = '0' + BUTTON_TODIGIT(ev->button);
				}
			}
		} else if (ev->button == BUTTON_DEL && e->flags.editing) {
			/* delete one char from input string */
			if (e->editPos <= 1) {
				/* string will be empty after deletion -> abort editing */
				e->flags.editing = 0;
			} else {
				e->editPos--;
				if (e->inputString[e->editPos] == '.') {
					/* deleted dot */
					e->flags.dotSet = 0;
				}
				e->inputString[e->editPos] = ' ';
			}
			widget_RequestRedraw(w);
		} else if (ev->button == BUTTON_ESC && e->flags.editing) {
			e->flags.editing = 0;
			widget_RequestRedraw(w);
		} else if ((ev->button & (BUTTON_UNIT1 | BUTTON_ENCODER | BUTTON_UNITm))
				&& e->flags.editing) {
			e->flags.editing = 0;
			/* TODO adjust multiplier to unit */
			uint32_t multiplier = 1000000;
			if (ev->button == BUTTON_UNITm) {
				multiplier = 1000;
			}
			int32_t newval = entry_GetInputStringValue(e, multiplier);
			*e->value = entry_constrainValue(e, newval);
			if (e->changeCallback) {
				e->changeCallback(w);
			}
			widget_RequestRedraw(w);
		}
		break;
    case EVENT_ENCODER_MOVED:
    	if(!e->flags.editing) {
			int32_t newval = *e->value += ev->movement
					* common_LeastDigitValueFromString(e->inputString, e->unit);
			*e->value = entry_constrainValue(e, newval);
			if(e->changeCallback) {
				e->changeCallback(w);
			}
			widget_RequestRedraw(w);
    	}
    	break;
    default:
    	break;
    }
    return;
}
