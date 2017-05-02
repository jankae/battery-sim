#include "dialog.h"

#define INPUT_DIALOG_LENGTH		10

struct {
	window_t *window;
	union {
		struct {
			char string[INPUT_DIALOG_LENGTH + 1];
			uint8_t pos;
			label_t *label;
			button_t *bdot;
			int32_t *ptr;
			int32_t min;
			int32_t max;
			const unit_t *unit;
		} input;
		struct {
			void (*cb)(MsgResult_t);
		} msgbox;
	};
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
	GUIEvent_t ev =
			{ .type = EVENT_WINDOW_CLOSE, .w = (widget_t*) dialog.window };
	gui_SendEvent(&ev);
}

void dialog_InputValue(int32_t * const result,
		const int32_t min, const int32_t max, const unit_t * const unit){
	/* check pointers */
	if (!result || !unit) {
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
	container_attach(c, (widget_t*) label, COORDS(0, 15));
	container_attach(c, (widget_t*) b7, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 0));
	container_attach(c, (widget_t*) b8, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 0));
	container_attach(c, (widget_t*) b9, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 0));
	container_attach(c, (widget_t*) b4, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 25));
	container_attach(c, (widget_t*) b5, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 25));
	container_attach(c, (widget_t*) b6, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 25));
	container_attach(c, (widget_t*) b1, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 50));
	container_attach(c, (widget_t*) b2, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 50));
	container_attach(c, (widget_t*) b3, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 50));
	container_attach(c, (widget_t*) b0, COORDS(KEY_OFFSET_X + 0, KEY_OFFSET_Y + 75));
	container_attach(c, (widget_t*) bdot, COORDS(KEY_OFFSET_X + 33, KEY_OFFSET_Y + 75));
	container_attach(c, (widget_t*) bdel, COORDS(KEY_OFFSET_X + 66, KEY_OFFSET_Y + 75));
	// TODO find a place for sign toggle
	container_attach(c, (widget_t*) bsign, COORDS(KEY_OFFSET_X - 33, KEY_OFFSET_Y + 50));
	/* unit selection buttons */
	uint8_t i = 0;
	/* position for first unit button is bottom left corner */
	uint8_t x = 1;
	uint8_t y = KEY_OFFSET_Y + 75;
	while((*unit)[i]) {
		/* got a unitElement_t */
		button_t *b = button_new((*unit)[i]->name, Font_Big, 41, inputDialog_Finished);
		container_attach(c, (widget_t*) b, COORDS(x, y));
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
	dialog.window = w;

	dialog.input.ptr = result;
	dialog.input.min = min;
	dialog.input.max = max;
	dialog.input.unit = unit;

	window_SetMainWidget(w, (widget_t*) c);

}

static void MessageBoxButton(widget_t *source) {
	button_t *b = (button_t*) source;
	MsgResult_t res = MSG_RESULT_ERR;
	/* find which button has been pressed */
	if(!strcmp(b->name, "OK")) {
		res = MSG_RESULT_OK;
	} else if(!strcmp(b->name, "ABORT")) {
		res = MSG_RESULT_ABORT;
	}

	if (dialog.msgbox.cb)
		dialog.msgbox.cb(res);

	// TODO does this always work? (might result in null pointer exception due to deleting
	// widgets while working on them)
	window_destroy((window_t*) dialog.window);

//	/* destroy dialog */
//	GUIEvent_t ev = { .type = EVENT_WINDOW_CLOSE, .w = dialog.window };
//	gui_SendEvent(&ev);
}

void dialog_MessageBox(const char * const title, const char * const msg,
		MsgBox_t type, void (*cb)(MsgResult_t)){
	/* check pointers */
	if (!title || !msg) {
		return;
	}
	/* create dialog window and elements */
	textfield_t *text = textfield_new(msg, Font_Medium, COORDS(300, 180));
	if (!text) {
		return;
	}
	/* calculate window size */
	coords_t windowSize = text->base.size;
	if (windowSize.x < 132) {
		windowSize.x = 136;
	} else {
		windowSize.x += 4;
	}
	windowSize.y += 50;
	window_t *w = window_new(title, Font_Big, windowSize);
	container_t *c = container_new(window_GetAvailableArea(w));
	container_attach(c, (widget_t*) text, COORDS(1, 2));
	switch (type) {
	case MSG_OK: {
		button_t *bOK = button_new("OK", Font_Big, 65, MessageBoxButton);
		container_attach(c, (widget_t*) bOK,
				COORDS((c->base.size.x - bOK->base.size.x) / 2,
						c->base.size.y - bOK->base.size.y - 1));
	}
		break;
	case MSG_ABORT_OK: {
		button_t *bOK = button_new("OK", Font_Big, 65, MessageBoxButton);
		button_t *bAbort = button_new("ABORT", Font_Big, 65, MessageBoxButton);
		container_attach(c, (widget_t*) bAbort,
				COORDS(c->base.size.x / 2 - bAbort->base.size.x - 1,
						c->base.size.y - bAbort->base.size.y - 1));
		container_attach(c, (widget_t*) bOK,
				COORDS(c->base.size.x / 2 + 1,
						c->base.size.y - bOK->base.size.y - 1));

	}
		break;
	}

	dialog.window = w;
	dialog.msgbox.cb = cb;

	window_SetMainWidget(w, (widget_t*) c);
}


