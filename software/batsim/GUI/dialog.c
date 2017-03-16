#include "dialog.h"

#define INPUT_DIALOG_LENGTH		10

union {
	struct {
		char string[INPUT_DIALOG_LENGTH + 1];
		uint8_t pos;
		label_t *label;
		button_t *bdot;
		window_t *window;
		int32_t *ptr;
		int32_t min;
		int32_t max;
		const unit_t *unit;
	} input;
} dialog;

static void inputDialog_AddChar(widget_t *source) {
	if (dialog.input.pos < 10) {
		/* still room in input */
		button_t *b = (button_t*) source;
		/* copy first char of button text */
		dialog.input.string[dialog.input.pos] = b->name[0];
		if (b->name[0] == '.') {
			/* prevent further dots */
			source->flags.selectable = 0;
		}
		dialog.input.pos++;
		/* text has changed */
		label_SetText(dialog.input.label, dialog.input.string);
	}
}

static void inputDialog_DeleteChar(widget_t *source) {
	if (dialog.input.pos > 0) {
		/* input available */
		dialog.input.pos--;
		if(dialog.input.string[dialog.input.pos]=='.') {
			/* deleted dot, allow new dot */
			dialog.input.bdot->base.flags.selectable = 1;
		}
		dialog.input.string[dialog.input.pos] = 0;
		/* text has changed */
		label_SetText(dialog.input.label, dialog.input.string);
	}
}

static void inputDialog_ToggleSign(widget_t *source) {
	if (dialog.input.string[0] == '-') {
		/* remove sign from string */
		memmove(&dialog.input.string[0], &dialog.input.string[1], dialog.input.pos);
		dialog.input.pos--;
	} else if(dialog.input.pos < 10) {
		/* space for sign available, add at the beginning */
		memmove(&dialog.input.string[1], &dialog.input.string[0], dialog.input.pos);
		/* add sign at the beginning */
		dialog.input.string[0] = '-';
		dialog.input.pos++;
	}
	/* text has changed */
	label_SetText(dialog.input.label, dialog.input.string);
}

static void inputDialog_Finished(widget_t *source) {
	button_t *b = (button_t*) source;
	uint32_t factor = 0;
	/* find unit of source button */
	uint8_t i = 0;
	while((*dialog.input.unit)[i]) {
		/* does the source button match this unit? */
		if(!strcmp((*dialog.input.unit)[i]->name, b->name)) {
			/* this unit was selected */
			factor = (*dialog.input.unit)[i]->factor;
			break;
		}
		i++;
	}
//	if(!factor) {
//		/* couldn't find unit. This should not be possible, as unit buttons
//		 * are created from the same array against which they are compared here */
//	}
	/* build value from string */
	int64_t value = 0;
	uint32_t div = 0;
	int8_t sign = 1;
	i = 0;
	if (dialog.input.string[0] == '-') {
		/* value is negative */
		sign = -1;
		i = 1;
	}
	for (; i < INPUT_DIALOG_LENGTH; i++) {
		if (!dialog.input.string[i]) {
			/* found end of string */
			break;
		} else if (dialog.input.string[i] == '.') {
			/* found dot */
			div = 1;
		} else {
			/* add digit to value */
			value *= 10;
			value += dialog.input.string[i] - '0';
			div *= 10;
		}
	}
	/* correct by unit factor */
	value *= factor;
	/* correct by dot position */
	if (div)
		value /= div;
	/* set sign */
	value *= sign;

	/* constrain value */
	if (value > dialog.input.max)
		value = dialog.input.max;
	else if (value < dialog.input.min)
		value = dialog.input.min;

	/* update value pointer */
	*dialog.input.ptr = (int32_t) value;

	/* destroy dialog */
	GUIEvent_t ev = { .type = EVENT_WINDOW_CLOSE, .w = dialog.input.window };
	gui_SendEvent(&ev);
}

void dialog_InputValue(const char * const title, int32_t * const result,
		const int32_t min, const int32_t max, const unit_t * const unit){
	/* check pointers */
	if (!title || !result || !unit) {
		return;
	}
	/* initialize input string */
	memset(dialog.input.string, 0, sizeof(dialog.input.string));
	dialog.input.pos = 0;
	/* create dialog window and elements */
	label_t *label = label_newWithLength(INPUT_DIALOG_LENGTH, Font_Big, LABEL_CENTER);
	/* keypad */
	button_t *b1 = button_new("1", Font_Big, 30, inputDialog_AddChar);
	button_t *b2 = button_new("2", Font_Big, 30, inputDialog_AddChar);
	button_t *b3 = button_new("3", Font_Big, 30, inputDialog_AddChar);
	button_t *b4 = button_new("4", Font_Big, 30, inputDialog_AddChar);
	button_t *b5 = button_new("5", Font_Big, 30, inputDialog_AddChar);
	button_t *b6 = button_new("6", Font_Big, 30, inputDialog_AddChar);
	button_t *b7 = button_new("7", Font_Big, 30, inputDialog_AddChar);
	button_t *b8 = button_new("8", Font_Big, 30, inputDialog_AddChar);
	button_t *b9 = button_new("9", Font_Big, 30, inputDialog_AddChar);
	button_t *b0 = button_new("0", Font_Big, 30, inputDialog_AddChar);
	button_t *bdot = button_new(".", Font_Big, 30, inputDialog_AddChar);
	button_t *bdel = button_new("\x1B", Font_Big, 30, inputDialog_DeleteChar);
	button_t *bsign = button_new("\xF1", Font_Big, 30, inputDialog_ToggleSign);
	window_t *w = window_new("New value:", Font_Big, SIZE(230, 120));
	container_t *c = container_new(window_GetAvailableArea(w));
#define KEY_OFFSET_X		130
#define KEY_OFFSET_Y		1
	container_attach(c, label, COORDS(0, 15));
	container_attach(c, b7, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 0));
	container_attach(c, b8, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 0));
	container_attach(c, b9, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 0));
	container_attach(c, b4, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 25));
	container_attach(c, b5, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 25));
	container_attach(c, b6, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 25));
	container_attach(c, b1, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 50));
	container_attach(c, b2, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 50));
	container_attach(c, b3, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 50));
	container_attach(c, b0, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 75));
	container_attach(c, bdot, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 75));
	container_attach(c, bdel, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 75));
	// TODO find a place for sign toggle
	container_attach(c, bsign, COORDS(KEY_OFFSET_X - 33, KEY_OFFSET_Y + 50));
	/* unit selection buttons */
	uint8_t i = 0;
	/* position for first unit button is bottom left corner */
	uint8_t x = 1;
	uint8_t y = KEY_OFFSET_Y + 75;
	while((*unit)[i]) {
		/* got a unitElement_t */
		button_t *b = button_new((*unit)[i]->name, Font_Big, 41, inputDialog_Finished);
		container_attach(c, b, COORDS(x, y));
		i++;
		/* position for next button is right of this button */
		x += 42;
		if (x >= KEY_OFFSET_X - 42) {
			/* reached keypad, move up one row */
			y -= 25;
			x = 1;
		}
	}

	/* save pointer to widgets */
	dialog.input.bdot = bdot;
	dialog.input.label = label;
	dialog.input.window = w;

	dialog.input.ptr = result;
	dialog.input.min = min;
	dialog.input.max = max;
	dialog.input.unit = unit;

	window_SetMainWidget(w, c);

}


