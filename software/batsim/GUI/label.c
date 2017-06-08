#include "label.h"

static label_t* label_newGeneric(void) {
	label_t* l = pvPortMalloc(sizeof(label_t));
	if (!l) {
		/* malloc failed */
		return NULL;
	}
	/* initialize common widget values */
	widget_init((widget_t*) l);
	/* set widget functions */
	l->base.func.draw = label_draw;
	l->base.func.input = label_input;
	l->color = LABEL_FG_COLOR;
	return l;
}

label_t* label_newWithLength(const uint8_t length, const font_t font,
		const labelOrient_t orient) {
	label_t* l = label_newGeneric();
	if (!l) {
		/* malloc failed */
		return NULL;
	}
	l->font = font;
	l->orient = orient;
	l->name[0] = 0;
	/* calculate size */
	l->base.size.y = font.height;
	l->base.size.x = font.width * length;

	return l;
}

label_t* label_newWithText(const char * const text, const font_t font) {
	label_t* l = label_newGeneric();
	if (!l) {
		/* malloc failed */
		return NULL;
	}
	l->font = font;
    uint8_t i = 0;
    while (text[i] && i < LABEL_MAX_NAME) {
        l->name[i] = text[i];
        i++;
    }
    l->name[i] = 0;
    l->base.flags.selectable = 0;
    /* calculate size */
    l->base.size.y = font.height;
    l->base.size.x = font.width * i;
    l->orient = LABEL_LEFT;
    l->fontStartX = 0;

    return l;
}

void label_SetText(label_t *l, const char * const text) {
	uint8_t i = 0;
	while (text[i] && (i + 1) * l->font.width <= l->base.size.x
			&& i < LABEL_MAX_NAME) {
		l->name[i] = text[i];
		i++;
	}
	l->name[i] = 0;
	/* set X position of font */
	switch (l->orient) {
	case LABEL_LEFT:
		/* always starting right at the label */
		l->fontStartX = 0;
		break;
	case LABEL_RIGHT:
		/* orient font on right side of label */
		l->fontStartX = l->base.size.x - i * l->font.width;
		break;
	case LABEL_CENTER:
		/* orient font in the middle */
		l->fontStartX = (l->base.size.x - i * l->font.width) / 2;
		break;
	}
	/* text has changed, this widget has to be drawn again */
	widget_RequestRedrawFull((widget_t*) l);
}

void label_draw(widget_t *w, coords_t offset) {
    label_t *l = (label_t*) w;
    display_SetForeground(l->color);
    display_SetBackground(LABEL_BG_COLOR);
	display_SetFont(l->font);
	display_String(offset.x + l->fontStartX, offset.y, l->name);
}

void label_input(widget_t *w, GUIEvent_t *ev) {
    /* label doesn't handle any input */
	return;
}
