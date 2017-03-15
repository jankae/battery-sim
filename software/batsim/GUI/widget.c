#include "widget.h"

void widget_init(widget_t *w) {
    memset(w, 0, sizeof(widget_t));
    w->flags.visible = 1;
    w->flags.selectable = 1;
    w->flags.redraw = 1;
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

GUIResult_t widget_draw(widget_t *w, coords_t pos) {
	if(!w)
		/* no widget given */
		return GUI_ERROR;
	if(!w->flags.redraw)
		/* this widget doesn't need a redraw */
		return GUI_OK;
	/* calculate new position */
	pos.x += w->position.x;
	pos.y += w->position.y;
	if (w->flags.redrawFull) {
		/* save foreground and set background as foreground */
		color_t fg = display_GetForeground();
		display_SetForeground(display_GetBackground());
		/* widget needs a full redraw, clear widget area */
		display_RectangleFull(pos.x, pos.y, pos.x + w->size.x - 1,
				pos.y + w->size.y - 1);
		/* clear flag */
		w->flags.redrawFull = 0;
		/* restore foreground */
		display_SetForeground(fg);
	}
	/* draw widget */
	GUIResult_t res = w->func.draw(w, pos);
	/* clear redraw request */
	w->flags.redraw = 0;
    return res;
}

void widget_input(widget_t *w, GUIEvent_t *ev) {
	switch(ev->type) {
	case GUI_TOUCH_PRESSED:
	case GUI_TOUCH_RELEASED:
		/* position based event */
		/* remove offset of own widget */
		ev->pos.x -= w->position.x;
		ev->pos.y -= w->position.y;
		/* first, try to handle it itself */
		w->func.input(w, ev);
		if(ev->type != GUI_EVENT_NONE) {
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
	case GUI_EVENT_NONE:
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
		if (first->firstChild)
			widget_RequestRedrawChildren(first->firstChild);
		first = first->next;
	}
	return GUI_OK;
}

GUIResult_t widget_RequestRedraw(widget_t *w) {
	if(!w)
		return GUI_ERROR;
	/* mark this widget */
	w->flags.redraw = 1;
	if(w->parent) {
		/* mark parent */
		return widget_RequestRedraw(w->parent);
	} else {
		return GUI_OK;
	}
}

GUIResult_t widget_RequestRedrawFull(widget_t *w) {
	if(!w)
		return GUI_ERROR;
	/* mark this widget */
	w->flags.redrawFull = 1;
	/* mark all children */
	if (w->firstChild)
		widget_RequestRedrawChildren(w->firstChild);
	/* mark parents */
	return widget_RequestRedraw(w);
}
