#include "widget.h"

void widget_init(widget_t *w) {
    memset(w, 0, sizeof(widget_t));
    w->flags.visible = 1;
    w->flags.selectable = 1;
    w->flags.redraw = 1;
}

/**
 * @brief Deletes a widget WITHOUT removing it from its parents list
 *
 * This function is only used internally when deleting widgets whos
 * parent will also be deleted.
 * @param w Widget to delete
 */
static void widget_deleteInt(widget_t *w) {
	/* delete children first as we still have the firstChild pointer */
	widget_t *next = w->firstChild;
	while(next) {
		/* save pointer to this widget */
		widget_t *this = next;
		/* save next widget in line (will get lost when this is freed) */
		next = this->next;
		/* delete and free this child */
		widget_deleteInt(this);
	}
	/* free memory of this widget */
	/* Although the exact size of the widget data is not known (widget_t is only the base data,
	 * the actual widget has a larger size (e.g. sizeof(button_t)), free still works because the
	 * size of the memory block associated with the widget is known by the memory management functions
	 */
	vPortFree(w);
}

void widget_delete(widget_t *w) {
	/* remove this widget from its parents list */
	if(w->parent) {
		/* remove widget from parent linked list */
		widget_t *it = w->parent->firstChild;
		/* which pointer is pointing to it */
		widget_t **pointer = &(w->parent->firstChild);
		while (it) {
			if (it == w) {
				/* found this widget */
				/* set pointer to point to next widget, removing this widget from the list */
				*pointer = it->next;
				/* widget found, loop is done */
				break;
			} else {
				/* not this widget, update it and pointer to it */
				pointer = &(it->next);
				it = it->next;
			}
		}
		/* request full redraw for parent */
		widget_RequestRedrawFull(w->parent);
	}
	/* delete the actually widget */
	widget_deleteInt(w);
}

GUIResult_t widget_selectNext(widget_t *first) {
    if (!first)
        return GUI_ERROR;
    widget_t *eligible = NULL;
    /* find selected item */
    while (first) {
        if (first->flags.selected)
            break;
        first = first->next;
    }
    if (!first) {
        /* no widget selected */
        return GUI_UNABLE;
    }
    /* find next selectable item */
    eligible = first->next;
    while (eligible) {
        if (eligible->flags.selectable && eligible->flags.visible)
            /* this item can be selected */
            break;
        eligible = eligible->next;
    }
    if (!eligible) {
        /* no next item selectable */
        return GUI_UNABLE;
    }
    /* select next item */
    first->flags.selected = 0;
    eligible->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_selectPrevious(widget_t *first) {
    if (!first)
        return GUI_ERROR;
    widget_t *eligible = NULL;
    /* find selected item */
    while (first) {
        if (first->flags.selected)
            break;
        if (first->flags.selectable && first->flags.visible)
            /* this widget could be selected */
            eligible = first;
        first = first->next;
    }
    if (!first) {
        /* no widget selected */
        return GUI_UNABLE;
    }
    if (!eligible) {
        /* no previous item selectable */
        return GUI_UNABLE;
    }
    /* select previous item */
    first->flags.selected = 0;
    eligible->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_selectFirst(widget_t *first) {
    if (!first)
        return GUI_ERROR;
    /* find first eligible item */
    while (first) {
        if (first->flags.selectable && first->flags.visible)
            break;
        first = first->next;
    }
    if (!first) {
        /* no widget eligible */
        return GUI_UNABLE;
    }
    /* select first eligible widget */
    first->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_selectWidget(widget_t *first, uint8_t num) {
    if (!first)
        return GUI_ERROR;
    while (first && num--) {
        first = first->next;
    }
    if (!first) {
        /* not enough widgets for num */
        return GUI_ERROR;
    }
    if (!first->flags.visible || !first->flags.selectable) {
        /* widget 'num' not selectable */
        return GUI_UNABLE;
    }
    first->flags.selected = 1;
    return GUI_OK;
}

GUIResult_t widget_deselectAll(widget_t *first) {
    if (!first)
        return GUI_ERROR;
    /* iterate over all widgets and deselect them */
    while (first) {
        first->flags.selected = 0;
        first = first->next;
    }
    return GUI_OK;
}

//void widget_gotFocus(widget_t *w) {
//    GUISignal_t s;
//    memset(&s, 0, sizeof(s));
//    s.gotFocus = 1;
//    w->func.input(w, s);
//}
//
//void widget_lostFocus(widget_t *w) {
//    GUISignal_t s;
//    memset(&s, 0, sizeof(s));
//    s.lostFocus = 1;
//    w->func.input(w, s);
//}

void widget_draw(widget_t *w, coords_t pos) {
	if (!w)
		/* no widget given */
		return;
	/* calculate new position */
	pos.x += w->position.x;
	pos.y += w->position.y;
	if (w->flags.redraw) {
		if (w->flags.redrawClear) {
			display_SetForeground(COLOR_BG_DEFAULT);
			/* widget needs a full redraw, clear widget area */
			display_RectangleFull(pos.x, pos.y, pos.x + w->size.x - 1,
					pos.y + w->size.y - 1);
			/* clear flag */
			w->flags.redrawClear = 0;
		}
		/* draw widget */
		w->func.draw(w, pos);
		/* clear redraw request */
		w->flags.redraw = 0;
	}
	if (w->flags.redrawChild) {
		/* draw children of this widget */
		w->func.drawChildren(w, pos);
		/* clear redraw request */
		w->flags.redrawChild = 0;
	}
}

void widget_input(widget_t *w, GUIEvent_t *ev) {
	switch(ev->type) {
	case EVENT_TOUCH_PRESSED:
	case EVENT_TOUCH_RELEASED:
		/* position based event */
		/* remove offset of own widget */
		ev->pos.x -= w->position.x;
		ev->pos.y -= w->position.y;
		/* first, try to handle it itself */
		w->func.input(w, ev);
		if(ev->type != EVENT_NONE) {
			/* event not handled yet */
			/* find matching child */
			widget_t *child = w->firstChild;
			for (; child; child = child->next) {
				if (child->flags.selectable) {
					/* potential candidate, check position */
					if (ev->pos.x >= child->position.x
							&& ev->pos.x <= child->position.x + child->size.x
							&& ev->pos.y >= child->position.y
							&& ev->pos.y <= child->position.y + child->size.y) {
						/* event is in child region */
						widget_input(child, ev);
						/* send event only to first match */
						return;
					}
				}
			}
		}
		break;
	default:
		break;
	}
//    /* First pass it on to any active child */
//    widget_t *child = w->firstChild;
//    for (; child; child = child->next) {
//        if (child->flags.selected) {
//            ev = widget_input(child, ev);
//        }
//    }
//    /* Then try to handle the event itself */
//    ev = w->func.input(w, ev);
//    return ev;
}

GUIResult_t widget_RequestRedrawChildren(widget_t *first){
    if (!first)
        return GUI_ERROR;
    /* iterate over all widgets and request a redraw them */
    while (first) {
        first->flags.redraw = 1;
		/* recursively request redraw of their children */
		if (first->firstChild) {
			/* widget got children itself */
			first->flags.redrawChild = 1;
			widget_RequestRedrawChildren(first->firstChild);
		}
		first = first->next;
	}
	return GUI_OK;
}

GUIResult_t widget_RequestRedraw(widget_t *w) {
	if(!w)
		return GUI_ERROR;
	/* mark this widget */
	w->flags.redraw = 1;
	while(w->parent) {
		/* this is not the top widget, indicate branch redraw */
		w = w->parent;
		if (w->flags.redrawChild) {
			/* reached a part that is already scheduled for redrawing */
			break;
		}
		w->flags.redrawChild = 1;
	}
	return GUI_OK;
}

GUIResult_t widget_RequestRedrawFull(widget_t *w) {
	if(!w)
		return GUI_ERROR;
	/* mark this widget */
	w->flags.redrawClear = 1;
	/* mark all children */
	if (w->firstChild) {
		w->flags.redrawChild = 1;
		widget_RequestRedrawChildren(w->firstChild);
	}
	/* mark parents */
	return widget_RequestRedraw(w);
}
