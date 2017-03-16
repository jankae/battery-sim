#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include "FreeRTOS.h"
#include "button.h"
#include "container.h"
#include "window.h"
#include "label.h"
#include "dialog.h"

void gui_Init(void);

void gui_SendEvent(GUIEvent_t *ev);

#endif
