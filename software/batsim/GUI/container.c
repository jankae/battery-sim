#include "container.h"

void container_create(container_t *c, int16_t width, int16_t height) {
    widget_init((widget_t*) c);
    c->base.size.x = width;
    c->base.size.y = height;
    c->base.func.draw = container_draw;
    c->base.func.input = container_input;
    c->flags.editing = 0;
    c->flags.focussed = 0;
    c->flags.scrollHorizontal = 0;
    c->flags.scrollVertical = 0;
    c->canvasSize.x = 0;
    c->canvasSize.y = 0;
    c->canvasOffset.x = 0;
    c->canvasOffset.y = 0;
    c->scrollBarLength.x = 0;
    c->scrollBarLength.y = 0;
    c->viewingSize = c->base.size;
}

GUIResult_t container_attach(container_t *c, widget_t *w, int16_t x, int16_t y) {
    if (c->base.firstChild) {
        /* find end of children list */
        widget_t *child = c->base.firstChild;
        while (child->next) {
            child = child->next;
        }
        /* add widget to the end */
        child->next = w;
    } else {
        /* this is the first child */
        c->base.firstChild = w;
    }
    w->position.x = x;
    w->position.y = y;
    w->parent = (widget_t*) c;

    /* extend canvas size if necessary */
    if (c->canvasSize.x < x + w->size.x) {
        c->canvasSize.x = x + w->size.x;
    }
    if (c->canvasSize.y < y + w->size.y) {
        c->canvasSize.y = y + w->size.y;
    }
    /* add scroll bars if necessary */
    if (c->canvasSize.x > c->base.size.x) {
        c->flags.scrollHorizontal = 1;
        c->viewingSize.y = c->base.size.y - CONTAINER_SCROLLBAR_SIZE;
    }
    if (c->canvasSize.y > c->viewingSize.y) {
        c->flags.scrollVertical = 1;
        c->viewingSize.x = c->base.size.x - CONTAINER_SCROLLBAR_SIZE;
        if (!c->flags.scrollHorizontal) {
            /* check again for horizontal scroll */
            if (c->canvasSize.x > c->viewingSize.x) {
                c->flags.scrollHorizontal = 1;
                c->viewingSize.y = c->base.size.y - CONTAINER_SCROLLBAR_SIZE;
            }
        }
    }
    /* adjust scroll bar sizes */
	if (c->flags.scrollHorizontal)
		c->scrollBarLength.x = common_Map(c->viewingSize.x, 0, c->canvasSize.x,
				0, c->viewingSize.x);
	if (c->flags.scrollVertical)
		c->scrollBarLength.y = common_Map(c->viewingSize.y, 0, c->canvasSize.y,
				0, c->viewingSize.y);
    return GUI_OK;
}

GUIResult_t container_draw(widget_t *w, coords_t offset) {
    container_t *c = (container_t*) w;
    GUIResult_t res = GUI_OK;
    widget_t *child = w->firstChild;
    widget_t *selected = child;
    for (; selected; selected = selected->next) {
        if (selected->flags.selected)
            break;
    }
    if (selected) {
        /* move canvas offset so that selected is visible */
        if (selected->position.x < c->canvasOffset.x) {
            c->canvasOffset.x = selected->position.x;
        } else if (selected->position.x + selected->size.x
                > c->viewingSize.x + c->canvasOffset.x) {
            c->canvasOffset.x = selected->position.x + selected->size.x
                    - c->viewingSize.x;
        }
        if (selected->position.y < c->canvasOffset.y) {
            c->canvasOffset.y = selected->position.y;
        } else if (selected->position.y + selected->size.y
                > c->viewingSize.y + c->canvasOffset.y) {
            c->canvasOffset.y = selected->position.y + selected->size.y
                    - c->viewingSize.y;
        }
    }
    /* draw scroll bars if necessary */
    if (c->flags.scrollVertical) {
        display_VerticalLine(
                offset.x + c->base.size.x - CONTAINER_SCROLLBAR_SIZE, offset.y,
                c->base.size.y);
        /* calculate beginning of scrollbar */
        uint8_t scrollBegin = common_Map(c->canvasOffset.y, 0, c->canvasSize.y,
                0,
                c->base.size.y
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollHorizontal);
		/* display position indicator */
		display_RectangleFull(
				offset.x + c->base.size.x - CONTAINER_SCROLLBAR_SIZE + 1,
				offset.y + scrollBegin, offset.x + c->base.size.x - 1,
				offset.y + scrollBegin + c->scrollBarLength.y - 1);
	}
    if (c->flags.scrollHorizontal) {
        display_HorizontalLine(offset.x,
                offset.y + c->base.size.y - CONTAINER_SCROLLBAR_SIZE,
                c->base.size.x);
        /* calculate beginning of scrollbar */
        uint8_t scrollBegin = common_Map(c->canvasOffset.x, 0, c->canvasSize.x,
                0,
                c->base.size.x
                        - CONTAINER_SCROLLBAR_SIZE * c->flags.scrollVertical);
		/* display position indicator */
		display_RectangleFull(offset.x + scrollBegin,
				offset.y + c->base.size.y - CONTAINER_SCROLLBAR_SIZE + 1,
				offset.x + scrollBegin + c->scrollBarLength.x - 1,
				offset.y + c->base.size.y - 1);
	}

	offset.x -= c->canvasOffset.x;
    offset.y -= c->canvasOffset.y;

    /* draw its children */
    for (; child; child = child->next) {
        if (res != GUI_OK) {
            /* abort on error */
            break;
        }
        if (child->flags.visible && !child->flags.selected) {
            /* check if child is fully in viewing field */
            if (child->position.x >= c->canvasOffset.x
                    && child->position.y >= c->canvasOffset.y
                    && child->position.x + child->size.x
                            <= c->canvasOffset.x + c->viewingSize.x
                    && child->position.y + child->size.y
                            <= c->canvasOffset.y + c->viewingSize.y) {
                /* draw this child */
                res = widget_draw(child, offset);
            }
        }
    }
    /* always draw selected child last (might overwrite other children) */
    if (selected) {
        res = widget_draw(selected, offset);
    }
    return res;
}

void container_input(widget_t *w, GUIEvent_t *ev) {
	container_t *c = (container_t*) w;
	switch(ev->type) {
	case GUI_TOUCH_PRESSED:
		if (ev->pos.x > c->viewingSize.y) {
			/* vertical scrollbar */
			/* adjust vertical canvas offset */
			c->canvasOffset.y = common_Map(ev->pos.y, c->scrollBarLength.y / 2,
					c->viewingSize.y - c->scrollBarLength.y / 2, 0,
					c->canvasSize.y - c->viewingSize.y);
			/* constrain offset */
			if (c->canvasOffset.y < 0)
				c->canvasOffset.y = 0;
			else if (c->canvasOffset.y > c->canvasSize.y - c->viewingSize.y)
				c->canvasOffset.y = c->canvasSize.y - c->viewingSize.y;
			/* clear event */
			ev->type = GUI_EVENT_NONE;
			/* request redraw */
			widget_RequestRedrawFull(w);
		} else if (ev->pos.y
				> w->size.y
						- c->flags.scrollHorizontal * CONTAINER_SCROLLBAR_SIZE) {
			/* horizontal scrollbar */
			// TODO
			/* clear event */
			ev->type = GUI_EVENT_NONE;
		}
		/* no break */
	case GUI_TOUCH_RELEASED:
		/* adjust position to canvas offset */
		ev->pos.x += c->canvasOffset.x;
		ev->pos.y += c->canvasOffset.y;
		break;
	default:
		break;
	}
}

