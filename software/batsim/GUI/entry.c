#include "entry.h"

entry_t* entry_new(int32_t *value, const int32_t *max, const int32_t *min,
		font_t font, uint8_t length, const unit_t * const unit) {
	entry_t* e = pvPortMalloc(sizeof(entry_t));
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
//    e->changeCallback = cb;
//    e->flags.editing = 0;
//    e->flags.encoderEdit = 0;
//    e->encEditPos = digits - 1;
	e->base.size.y = font.height + 3;
	e->base.size.x = font.width * length + 3;

	return e;
}

//uint32_t entry_GetInputStringValue(entry_t *e, uint32_t multiplier) {
//    uint32_t value = 0;
//    uint8_t i;
//    uint32_t div = 0;
//    for (i = 0; i < e->digits + 1; i++) {
//        if (e->inputString[i] >= '0' && e->inputString[i] <= '9') {
//            value *= 10;
//            value += e->inputString[i] - '0';
//            if (div) {
//                div *= 10;
//            }
//        } else if (e->inputString[i] == '.') {
//            div = 1;
//        }
//    }
//    value *= multiplier;
//    if (div)
//        value /= div;
//    return value;
//}
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
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

	/* construct value string */
	char s[e->length+1];
	common_StringFromValue(s, e->length, *e->value, e->unit);
	/* display string */
	display_String(upperLeft.x + 1, upperLeft.y + 2, s);
}

void entry_input(widget_t *w, GUIEvent_t *ev) {
    entry_t *e = (entry_t*) w;
    if (ev->type == EVENT_TOUCH_RELEASED) {
    	dialog_InputValue("New value:", e->value, *e->min, *e->max, e->unit);
    	widget_RequestRedraw(w);
		ev->type = EVENT_NONE;
	}
    return;
}
