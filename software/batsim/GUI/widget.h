#ifndef GUI_WIDGET_H_
#define GUI_WIDGET_H_

#include <string.h>
#include <stdint.h>

#include "events.h"
#include "display.h"

typedef struct widgetFunctions widgetFunctions_t;
typedef struct widget widget_t;

struct widgetFunctions {
    GUIResult_t (*draw)(widget_t *w, coords_t offset);
    void (*input)(widget_t *w, GUIEvent_t *ev);
};

struct widget {
    widget_t *parent;
    widget_t *firstChild;
    widget_t *next;

    coords_t position;
    coords_t size;
//    coords_t childrenOffset;

    struct {
        uint8_t visible :1;
        uint8_t selected :1;
        uint8_t selectable :1;
        uint8_t focus :1;
        uint8_t redraw :1;
        uint8_t redrawFull :1;
    } flags;

    widgetFunctions_t func;
};

void widget_init(widget_t *w);
GUIResult_t widget_selectNext(widget_t *first);
GUIResult_t widget_selectPrevious(widget_t *first);
GUIResult_t widget_selectFirst(widget_t *first);
GUIResult_t widget_selectWidget(widget_t *first, uint8_t num);
GUIResult_t widget_deselectAll(widget_t *first);
void widget_gotFocus(widget_t *w);
void widget_lostFocus(widget_t *w);
GUIResult_t widget_draw(widget_t *w, coords_t offset);
void widget_input(widget_t *w, GUIEvent_t *ev);
GUIResult_t widget_RequestRedrawChildren(widget_t *first);
GUIResult_t widget_RequestRedraw(widget_t *w);
GUIResult_t widget_RequestRedrawFull(widget_t *w);

inline void widget_SetVisible(widget_t *w) {
    w->flags.visible = 1;
}
inline void widget_SetInvisible(widget_t *w) {
    w->flags.visible = 0;
}

#endif
