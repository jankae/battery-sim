#include "keyboard.h"

#include "buttons.h"

#define LAYOUT_X		10
#define LAYOUT_Y		4

static const char lowerLayout[LAYOUT_Y][LAYOUT_X] = {
		{'1', '2','3','4','5','6','7','8','9','0',},
		{'q', 'w','e','r','t','y','u','i','o','p',},
		{'a', 's','d','f','g','h','j','k','l',';',},
		{'z', 'x','c','v','b','n','m',',','.','-',},
};
static const char upperLayout[LAYOUT_Y][LAYOUT_X] = {
		{'!', '@','#','$','%','^','&','*','(',')',},
		{'Q', 'W','E','R','T','Y','U','I','O','P',},
		{'A', 'S','D','F','G','H','J','K','L',':',},
		{'Z', 'X','C','V','B','N','M','<','>','_',},
};

keyboard_t* keyboard_new(void (*cb)(char)) {
	keyboard_t* k = pvPortMalloc(sizeof(keyboard_t));
	if (!k) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) k);
    /* set widget functions */
    k->base.func.draw = keyboard_draw;
    k->base.func.input = keyboard_input;
    /* a textfield can't have any children */
    k->base.func.drawChildren = NULL;

    k->keyPressedCallback = cb;
    k->selectedX = 0;
    k->selectedY = 0;
    k->shift = 0;

    k->base.size.x = LAYOUT_X * KEYBOARD_SPACING_X + 1;
    k->base.size.y = (LAYOUT_Y + 1) * KEYBOARD_SPACING_Y + 1;

    return k;
}
void keyboard_draw(widget_t *w, coords_t offset) {
	keyboard_t *k = (keyboard_t*) w;

	/* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += k->base.size.x - 1;
    lowerRight.y += k->base.size.y - 1;

    /* Draw surrounding rectangle */
	if (w->flags.selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground(KEYBOARD_BORDER_COLOR);
	}
    display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
	display_SetForeground(KEYBOARD_BORDER_COLOR);

	/* display dividing lines */
	display_SetForeground(KEYBOARD_LINE_COLOR);
	uint8_t i;
	for (i = 1; i <= LAYOUT_Y; i++) {
		display_HorizontalLine(upperLeft.x + 1,
				upperLeft.y + i * KEYBOARD_SPACING_Y, w->size.x - 2);
	}
	for (i = 1; i < LAYOUT_X; i++) {
		uint8_t length = LAYOUT_Y * KEYBOARD_SPACING_Y - 1;
		if (i == 3 || i == LAYOUT_X - 3) {
			length += KEYBOARD_SPACING_Y;
		}
		display_VerticalLine(upperLeft.x + i * KEYBOARD_SPACING_X,
				upperLeft.y + 1, length);
	}

	display_SetBackground(KEYBOARD_BG_COLOR);
	/* Draw keys */
	for (i = 0; i < LAYOUT_Y; i++) {
		uint8_t j;
		for (j = 0; j < LAYOUT_X; j++) {
			if (i == k->selectedY && j == k->selectedX) {
				/* this is the selected key */
				display_SetForeground(KEYBOARD_SELECTED_COLOR);
			} else {
				display_SetForeground(KEYBOARD_BORDER_COLOR);
			}
			char c = lowerLayout[i][j];
			if (k->shift) {
				c = upperLayout[i][j];
			}
			display_Char(
					upperLeft.x + j * KEYBOARD_SPACING_X
							+ (KEYBOARD_SPACING_X - KEYBOARD_FONT.width) / 2,
					upperLeft.y + i * KEYBOARD_SPACING_Y
							+ (KEYBOARD_SPACING_Y - KEYBOARD_FONT.height) / 2,
					c);
		}
	}

	/* Draw bottom bar with shift, space and backspace */
	/* SHIFT */
	if(k->selectedY == LAYOUT_Y && k->selectedX<3) {
		/* shift is selected */
		display_SetForeground(KEYBOARD_SELECTED_COLOR);
	} else if(k->shift){
		/* shift is not selected, but active */
		display_SetForeground(KEYBOARD_SHIFT_ACTIVE_COLOR);
	} else {
		display_SetForeground(KEYBOARD_BORDER_COLOR);
	}
	display_String(
			upperLeft.x
					+ (3 * KEYBOARD_SPACING_X - 5 * KEYBOARD_FONT.width) / 2,
			upperLeft.y + LAYOUT_Y * KEYBOARD_SPACING_Y
					+ (KEYBOARD_SPACING_Y - KEYBOARD_FONT.height) / 2, "SHIFT");

	/* SPACE */
	if (k->selectedY == LAYOUT_Y && k->selectedX >= 3
			&& k->selectedX < LAYOUT_X - 3) {
		/* space is selected */
		display_SetForeground(KEYBOARD_SELECTED_COLOR);
	} else {
		display_SetForeground(KEYBOARD_BORDER_COLOR);
	}
	display_String(
			upperLeft.x
					+ ((LAYOUT_X - 6) * KEYBOARD_SPACING_X
							- 5 * KEYBOARD_FONT.width)
							/ 2+ 3*KEYBOARD_SPACING_X,
			upperLeft.y + LAYOUT_Y * KEYBOARD_SPACING_Y
					+ (KEYBOARD_SPACING_Y - KEYBOARD_FONT.height) / 2, "SPACE");

	/* DEL */
	if (k->selectedY == LAYOUT_Y && k->selectedX >= LAYOUT_X - 3) {
		/* del is selected */
		display_SetForeground(KEYBOARD_SELECTED_COLOR);
	} else {
		display_SetForeground(KEYBOARD_BORDER_COLOR);
	}
	display_String(
			upperLeft.x
					+ (3 * KEYBOARD_SPACING_X - 3 * KEYBOARD_FONT.width)
							/ 2+ (LAYOUT_X-3)*KEYBOARD_SPACING_X,
			upperLeft.y + LAYOUT_Y * KEYBOARD_SPACING_Y
					+ (KEYBOARD_SPACING_Y - KEYBOARD_FONT.height) / 2, "DEL");
}

static void sendChar(keyboard_t *k) {
	if(k->selectedY < LAYOUT_Y) {
		/* a character key is selected */
		if (k->keyPressedCallback) {
			char c = lowerLayout[k->selectedY][k->selectedX];
			if (k->shift) {
				c = upperLayout[k->selectedY][k->selectedX];
			}
			k->keyPressedCallback(c);
		}
	} else {
		if(k->selectedX <3) {
			/* shift is selected */
			k->shift = !k->shift;
			widget_RequestRedraw((widget_t*) k);
		} else if(k->selectedX >= LAYOUT_X - 3) {
			/* del is selected */
			if (k->keyPressedCallback) {
				k->keyPressedCallback(0x08);
			}
		} else {
			/* space is selected */
			if (k->keyPressedCallback) {
				k->keyPressedCallback(' ');
			}
		}
	}
}

void keyboard_input(widget_t *w, GUIEvent_t *ev) {
	keyboard_t *k = (keyboard_t*) w;

	switch(ev->type) {
	case EVENT_ENCODER_MOVED:
		if (ev->movement > 0) {
			/* move to the right, ignore amount of movement */
			k->selectedX++;
			if (k->selectedX >= LAYOUT_X) {
				k->selectedX = 0;
				k->selectedY++;
				if (k->selectedY > LAYOUT_Y) {
					k->selectedY = 0;
				}
			}
		} else {
			/* move to the left, ignore amount of movement */
			if (k->selectedX > 0) {
				k->selectedX--;
			} else {
				k->selectedX = LAYOUT_X - 1;
				if (k->selectedY > 0) {
					k->selectedY--;
				} else {
					k->selectedY = LAYOUT_Y;
				}
			}
		}
		widget_RequestRedraw(w);
		break;
	case EVENT_BUTTON_CLICKED:
		if (ev->button & (BUTTON_ENTER | BUTTON_ENCODER)) {
			sendChar(k);
		} else if (BUTTON_IS_DIGIT(ev->button)) {
			/* send this digit directly */
			if (k->keyPressedCallback) {
				k->keyPressedCallback(BUTTON_TODIGIT(ev->button) + '0');
			}
		} else if (BUTTON_IS_ARROW(ev->button)) {
			switch (ev->button) {
			case BUTTON_LEFT:
				if (k->selectedX > 0) {
					k->selectedX--;
				} else {
					k->selectedX = LAYOUT_X - 1;
				}
				break;
			case BUTTON_RIGHT:
				if (k->selectedX < LAYOUT_X - 1) {
					k->selectedX++;
				} else {
					k->selectedX = 0;
				}
				break;
			case BUTTON_UP:
				if (k->selectedY > 0) {
					k->selectedY--;
				} else {
					k->selectedY = LAYOUT_Y;
				}
				break;
			case BUTTON_DOWN:
				if (k->selectedY < LAYOUT_Y) {
					k->selectedY++;
				} else {
					k->selectedY = 0;
				}
				break;
			}
			widget_RequestRedraw(w);
		} else if(ev->button == BUTTON_DEL) {
			if (k->keyPressedCallback) {
				k->keyPressedCallback(0x08);
			}
		}
		break;
	case EVENT_TOUCH_PRESSED:
		if (w->flags.selected) {
			/* only react to touch if already selected. This allows
			 * the user to select the widget without already changing the value */
			/* Calculate key pressed */
			k->selectedX = ev->pos.x / KEYBOARD_SPACING_X;
			k->selectedY = ev->pos.y / KEYBOARD_SPACING_Y;
			sendChar(k);
			widget_RequestRedraw(w);
		}
		break;
	default:
		break;
	}
	return;
}
