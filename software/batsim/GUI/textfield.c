#include "textfield.h"

textfield_t* textfield_new(const char *text, const font_t font,
		coords_t maxSize){
	textfield_t* t = pvPortMalloc(sizeof(textfield_t));
	if (!t) {
		/* malloc failed */
		return NULL;
	}
    /* initialize common widget values */
    widget_init((widget_t*) t);
    /* set widget functions */
    t->base.func.draw = textfield_draw;
    t->base.func.input = textfield_input;
    /* a textfield can't have any children */
    t->base.func.drawChildren = NULL;

    t->font = font;

	/* extract necessary size from text */
	uint16_t maxWidth = 0;
	uint16_t height = font.height;
	uint16_t width = 0;
	char *s = text;
	while (*text) {
		if (*text == '\n') {
			height += font.height;
			width = 0;
		} else {
			width += font.width;
			if (width > maxWidth) {
				maxWidth = width;
			}
		}
		if (height > maxSize.y || maxWidth > maxSize.x) {
			/* text exceeds maximum widget size */
			/* abort widget creation */
			vPortFree(t);
			return NULL;
		}
		text++;
	}
	/* set widget size */
	t->base.size.x = maxWidth;
	t->base.size.y = height;
	/* copy text */
	t->text = pvPortMalloc(strlen(s));
	if(!t->text) {
		/* malloc failed */
		/* abort widget creation */
		vPortFree(t);
		return NULL;
	}
	strcpy(t->text, s);

	return t;
}
void textfield_draw(widget_t *w, coords_t offset) {
	textfield_t *t = (textfield_t*) w;
	char *s = t->text;
	coords_t pos = offset;
	display_SetForeground(TEXTFIELD_FG_COLOR);
	display_SetBackground(TEXTFIELD_BG_COLOR);
	display_SetFont(t->font);
	while (*s) {
		if (*s == '\n') {
			pos.x = offset.x;
			pos.y += t->font.height;
		} else {
			display_Char(pos.x, pos.y, *s);
			pos.x += t->font.width;
		}
		s++;
	}
}
void textfield_input(widget_t *w, GUIEvent_t *ev){
    /* textfield doesn't handle any input */
	return;
}
