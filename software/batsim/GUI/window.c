#include "window.h"

extern widget_t *topWidget;

window_t* window_new(const char *titel, font_t font, coords_t size) {
	window_t *w = pvPortMalloc(sizeof(window_t));
	if (!w) {
		/* malloc failed */
		return NULL;
	}
	widget_init((widget_t*) w);
	w->base.size = size;
	/* center window on screen */
	w->base.position.x = (DISPLAY_WIDTH - size.x) / 2;
	w->base.position.y = (DISPLAY_HEIGHT - size.y) / 2;
	w->base.func.draw = window_draw;
	w->base.func.input = window_input;
	w->base.func.drawChildren = window_drawChildren;
	/* a new widget must have its area cleared */
	w->base.flags.redrawClear = 1;
	w->font = font;
	/* set title */
	uint8_t i = 0;
	while (*titel && i < WINDOW_MAX_NAME) {
		w->title[i++] = *titel++;
	}
	w->title[i] = 0;
	/* store last top widget (will be restored when this window closes) */
	w->lastTopWidget = topWidget;
	topWidget = (widget_t*) w;

	return w;
}

void window_destroy(window_t *w) {
	/* restore last top widget */
	topWidget = w->lastTopWidget;
	/* delete this window and all its sub-widgets */
	widget_delete((widget_t*) w);
	/* request full redraw of new top widget */
	widget_RequestRedrawFull(topWidget);
}

GUIResult_t window_SetMainWidget(window_t *w, widget_t *widg) {
	if(w->base.firstChild) {
		/* window already has a widget in it */
		return GUI_ERROR;
	}
	coords_t maxSize = window_GetAvailableArea(w);
	if(widg->size.x > maxSize.x || widg->size.y > maxSize.y) {
		/* widget doesn't fit in window */
		// TODO this potentially allows for a memory leak if the application doesn't check for
		// a return code. The widget (which failed to be attached to the window) won't get freed
		// when the window is closed.
		return GUI_ERROR;
	}
	w->base.firstChild = widg;
	w->base.flags.redrawChild = 1;
	widg->parent = (widget_t*) w;
	/* set child offset */
	widg->position.x = 1;
	widg->position.y = w->font.height + 4;

	return GUI_OK;
}
void window_draw(widget_t *w, coords_t offset) {
	window_t *window = (window_t*) w;
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += w->size.x - 1;
    lowerRight.y += w->size.y - 1;
	display_SetForeground(WINDOW_BORDER_COLOR);
    /* draw outline */
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
	/* draw dividing line under the title bar */
	display_HorizontalLine(upperLeft.x + 1, upperLeft.y + window->font.height + 3, w->size.x - 2);
	/* draw dividing line between title and close button */
	display_VerticalLine(upperLeft.x + window->font.height + 3, upperLeft.y + 1,
			window->font.height + 2);
	/* fill close area with background color */
	display_SetForeground(WINDOW_CLOSE_AREA_COLOR);
	display_RectangleFull(upperLeft.x + 1, upperLeft.y + 1,
			upperLeft.x + window->font.height + 2,
			upperLeft.y + window->font.height + 2);
	/* draw X in close area */
	display_SetForeground(WINDOW_CLOSE_X_COLOR);
	display_Line(upperLeft.x + 2, upperLeft.y + 2,
			upperLeft.x + window->font.height + 1,
			upperLeft.y + window->font.height + 1);
	display_Line(upperLeft.x + 2, upperLeft.y + window->font.height + 1,
			upperLeft.x + window->font.height + 1, upperLeft.y + 2);
	/* fill title bar with background color */
	display_SetForeground(WINDOW_TITLE_BG_COLOR);
	display_RectangleFull(upperLeft.x + window->font.height + 4,
			upperLeft.y + 1, lowerRight.x - 1,
			upperLeft.y + window->font.height + 2);
	/* add title */
	display_SetForeground(WINDOW_TITLE_FG_COLOR);
	display_SetBackground(WINDOW_TITLE_BG_COLOR);
	display_String(upperLeft.x + window->font.height + 5, upperLeft.y + 2,
			window->title);
}

void window_drawChildren(widget_t *w, coords_t offset) {
	if (w->firstChild) {
		widget_draw(w->firstChild, offset);
	}
}

coords_t window_GetAvailableArea(window_t *w) {
	return SIZE(w->base.size.x - 2, w->base.size.y - w->font.height - 5);
}

void window_input(widget_t *w, GUIEvent_t *ev) {
	window_t *window = (window_t*) w;
	switch (ev->type) {
	case GUI_TOUCH_PRESSED:
	case GUI_TOUCH_RELEASED:
		if (ev->pos.y <= window->font.height + 3) {
			/* mark event as handled */
			ev->type = GUI_EVENT_NONE;
			if (ev->pos.x <= window->font.height + 3) {
				/* clicked into window close area, close this window */
				window_destroy(w);
			}
		}
		break;
	default:
		break;
	}
}
