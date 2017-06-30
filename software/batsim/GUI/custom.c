#include "custom.h"

widget_t* custom_new(coords_t size, void (*draw)(widget_t *, coords_t),
		void (*input)(widget_t *, GUIEvent_t *)) {
	widget_t *c = pvPortMalloc(sizeof(widget_t));
	if (!c) {
		/* malloc failed */
		return NULL;
	}
	/* initialize common widget values */
	widget_init(c);
	/* set widget functions */
	c->func.draw = draw;
	if (input) {
		c->func.input = input;
	} else {
		/* If no input function is given, use dummy function pointer */
		c->func.input = custom_dummyInput;
	}
	c->size = size;

	return c;
}

void custom_dummyInput(widget_t *w, GUIEvent_t *ev) {
	return;
}



