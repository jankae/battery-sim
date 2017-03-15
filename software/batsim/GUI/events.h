#ifndef GUI_EVENTS_H_
#define GUI_EVENTS_H_

#include "common.h"

typedef enum  {GUI_OK, GUI_ERROR, GUI_UNABLE} GUIResult_t;

typedef struct event GUIEvent_t;

typedef enum {GUI_EVENT_NONE, GUI_TOUCH_PRESSED, GUI_TOUCH_RELEASED} GUIEventType_t;

struct event {
    /* indicates the type of event */
	GUIEventType_t type;
	/* data corresponding to the event type */
    union {
		coords_t pos;
    };
};

#endif
