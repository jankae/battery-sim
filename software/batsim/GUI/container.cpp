#include "container.h"

Container::Container(coords_t size) {
	this->size = size;
	editing = false;
	focussed = false;
	scrollHorizontal = false;
	scrollVertical = false;
	canvasSize.x = 0;
	canvasSize.y = 0;
	canvasOffset.x = 0;
	canvasOffset.y = 0;
	scrollBarLength.x = 0;
	scrollBarLength.y = 0;
	viewingSize = size;
}

void Container::attach(Widget *w, coords_t offset) {
	if (firstChild) {
		/* find end of children list */
		Widget *child = firstChild;
		do {
			if (child == w) {
				/* this widget has already been added, this must never happen */
				CRIT_ERROR("Duplicate widget in container");
			}
			if (child->next) {
				child = child->next;
			} else {
				break;
			}
		} while (1);
		/* add widget to the end */
		child->next = w;
	} else {
		/* this is the first child */
		firstChild = w;
	}
	w->position = offset;
	redrawChild = true;
	w->parent = this;

	/* extend canvas size if necessary */
	if (canvasSize.x < position.x + w->size.x) {
		canvasSize.x = position.x + w->size.x;
	}
	if (canvasSize.y < position.y + w->size.y) {
		canvasSize.y = position.y + w->size.y;
	}
	/* add scroll bars if necessary */
	if (canvasSize.x > size.x) {
		scrollHorizontal = true;
		viewingSize.y = size.y - ScrollbarSize;
	}
	if (canvasSize.y > viewingSize.y) {
		scrollVertical = true;
		viewingSize.x = size.x - ScrollbarSize;
		if (!scrollHorizontal) {
			/* check again for horizontal scroll */
			if (canvasSize.x > viewingSize.x) {
				scrollHorizontal = true;
				viewingSize.y = size.y - ScrollbarSize;
			}
		}
	}
	/* adjust scroll bar sizes */
	if (scrollHorizontal)
		scrollBarLength.x = common_Map(viewingSize.x, 0, canvasSize.x, 0,
				viewingSize.x);
	if (scrollVertical)
		scrollBarLength.y = common_Map(viewingSize.y, 0, canvasSize.y, 0,
				viewingSize.y);
}

void Container::draw(coords_t offset) {
	Widget *child = firstChild;
	Widget *selected = child;
	for (; selected; selected = selected->next) {
		if (selected->selected) {
			/* move canvas offset so that selected is visible */
			if (selected->position.x < canvasOffset.x) {
				canvasOffset.x = selected->position.x;
			} else if (selected->position.x + selected->size.x
					> viewingSize.x + canvasOffset.x) {
				canvasOffset.x = selected->position.x + selected->size.x
						- viewingSize.x;
			}
			if (selected->position.y < canvasOffset.y) {
				canvasOffset.y = selected->position.y;
			} else if (selected->position.y + selected->size.y
					> viewingSize.y + canvasOffset.y) {
				canvasOffset.y = selected->position.y + selected->size.y
						- viewingSize.y;
			}
		}
		break;
	}
	/* draw scroll bars if necessary */
	if (scrollVertical) {
		display_SetForeground (LineColor);
		display_VerticalLine(offset.x + size.x - ScrollbarSize, offset.y,
				size.y);
		/* calculate beginning of scrollbar */
		uint8_t scrollBegin = common_Map(canvasOffset.y, 0, canvasSize.y, 0,
				size.y - ScrollbarSize * scrollHorizontal);
		/* display position indicator */
		display_SetForeground (ScrollbarColor);
		display_RectangleFull(offset.x + size.x - ScrollbarSize + 1,
				offset.y + scrollBegin, offset.x + size.x - 1,
				offset.y + scrollBegin + scrollBarLength.y - 1);
	}
	if (scrollHorizontal) {
		display_SetForeground (LineColor);
		display_HorizontalLine(offset.x, offset.y + size.y - ScrollbarSize,
				size.x);
		/* calculate beginning of scrollbar */
		uint8_t scrollBegin = common_Map(canvasOffset.x, 0, canvasSize.x, 0,
				size.x - ScrollbarSize * scrollVertical);
		/* display position indicator */
		display_SetForeground (ScrollbarColor);
		display_RectangleFull(offset.x + scrollBegin,
				offset.y + size.y - ScrollbarSize + 1,
				offset.x + scrollBegin + scrollBarLength.x - 1,
				offset.y + size.y - 1);
	}
}

void Container::input(GUIEvent_t *ev) {
	switch (ev->type) {
	case EVENT_TOUCH_PRESSED: {
		/* save old canvasOffset */
		coords_t old = canvasOffset;
		if (ev->pos.x > viewingSize.x) {
			/* vertical scrollbar */
			/* adjust vertical canvas offset */
			canvasOffset.y = common_Map(ev->pos.y, scrollBarLength.y / 2,
					viewingSize.y - scrollBarLength.y / 2, 0,
					canvasSize.y - viewingSize.y);
			/* constrain offset */
			if (canvasOffset.y < 0)
				canvasOffset.y = 0;
			else if (canvasOffset.y > canvasSize.y - viewingSize.y)
				canvasOffset.y = canvasSize.y - viewingSize.y;
			/* clear event */
			ev->type = EVENT_NONE;
		} else if (ev->pos.y > size.y - scrollHorizontal * ScrollbarSize) {
			/* horizontal scrollbar */
			/* adjust horizontal canvas offset */
			canvasOffset.x = common_Map(ev->pos.x, scrollBarLength.x / 2,
					viewingSize.x - scrollBarLength.x / 2, 0,
					canvasSize.x - viewingSize.x);
			/* constrain offset */
			if (canvasOffset.x < 0)
				canvasOffset.x = 0;
			else if (canvasOffset.x > canvasSize.x - viewingSize.x)
				canvasOffset.x = canvasSize.x - viewingSize.x;
			/* clear event */
			ev->type = EVENT_NONE;
		}
		/* check if canvas moved */
		if (canvasOffset.x != old.x || canvasOffset.y != old.y) {
			/* request redraw */
			requestRedrawFull();
		}
	}
		/* no break */
	case EVENT_TOUCH_RELEASED:
		/* adjust position to canvas offset */
		ev->pos.x += canvasOffset.x;
		ev->pos.y += canvasOffset.y;
		break;
	default:
		break;
	}
}

void Container::drawChildren(coords_t offset) {
    Widget *child = firstChild;
    Widget *selected = child;
    for (; selected; selected = selected->next) {
        if (selected->selected)
            break;
    }

	offset.x -= canvasOffset.x;
    offset.y -= canvasOffset.y;

    /* draw its children */
    for (; child; child = child->next) {
         if (child->visible && !child->selected) {
            /* check if child is fully in viewing field */
            if (child->position.x >= canvasOffset.x
                    && child->position.y >= canvasOffset.y
                    && child->position.x + child->size.x
                            <= canvasOffset.x + viewingSize.x
                    && child->position.y + child->size.y
                            <= canvasOffset.y + viewingSize.y) {
                /* draw this child */
                Widget::draw(child, offset);
            }
        }
    }
    /* always draw selected child last (might overwrite other children) */
    if (selected) {
    	Widget::draw(selected, offset);
    }

}
