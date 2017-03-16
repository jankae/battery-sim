#ifndef GUI_EVENTS_H_
#define GUI_EVENTS_H_

#include "common.h"
#include "widget.h"

typedef enum  {GUI_OK, GUI_ERROR, GUI_UNABLE} GUIResult_t;

typedef struct event GUIEvent_t;

typedef enum {EVENT_NONE, EVENT_TOUCH_PRESSED, EVENT_TOUCH_RELEASED, EVENT_WINDOW_CLOSE} GUIEventType_t;

struct event {
    /* indicates the type of event */
	GUIEventType_t type;
	/* data corresponding to the event type */
    union {
		/* coordinates for position based events */
		coords_t pos;
		/* pointer to widget for widget based events */
		widget_t *w;
    };
};

#endif
