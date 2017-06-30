#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "widget.h"

widget_t* custom_new(coords_t size, void (*draw)(widget_t *, coords_t), void (*input)(widget_t *, GUIEvent_t *));
void custom_dummyInput(widget_t *w, GUIEvent_t *ev);

#endif
