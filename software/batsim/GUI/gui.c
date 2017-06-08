#include "gui.h"

#include "buttons.h"
#include "pushpull.h"

QueueHandle_t eventQueue = NULL;
widget_t *topWidget;
uint8_t isPopup;

TaskHandle_t GUIHandle;

extern widget_t *selectedWidget;

static void guiThread(void) {
	GUIHandle = xTaskGetCurrentTaskHandle();

	topWidget = NULL;
	GUIEvent_t event;

	display_SetBackground(COLOR_BLACK);
	display_Clear();

	desktop_Draw();

	while (1) {
		if (xQueueReceive(eventQueue, &event, 100)) {
			if (topWidget) {
				switch (event.type) {
				case EVENT_TOUCH_PRESSED:
				case EVENT_TOUCH_RELEASED:
				case EVENT_TOUCH_HELD:
					/* these are position based events */
					/* check if applicable for top widget
					 * (which, for smaller windows, might not be the case */
					if (event.pos.x >= topWidget->position.x
							&& event.pos.y >= topWidget->position.y
							&& event.pos.x
									< topWidget->position.x + topWidget->size.x
							&& event.pos.y
									< topWidget->position.y
											+ topWidget->size.y) {
						/* send event to top widget */
						widget_input(topWidget, &event);
					} else if (!isPopup) {
						// TODO block desktop input if topwidget is a popup window
						desktop_Input(&event);
					}
					break;
				case EVENT_BUTTON_CLICKED:
					if(event.button == BUTTON_ONOFF) {
						/* this is a special case button that is always relayed to the
						 * App in control of the output stage */
						if (pushpull_GetControlHandle()) {
							xTaskNotify(pushpull_GetControlHandle(),
									SIGNAL_ONOFF_BUTTON,
									eSetBits);
						}
						break;
					}
					/* no break */
				case EVENT_ENCODER_MOVED:
					/* these events are always valid for the selected widget */
					if (selectedWidget) {
						selectedWidget->func.input(selectedWidget, &event);
					} else {
						desktop_Input(&event);
					}
					break;
				case EVENT_WINDOW_CLOSE:
					window_destroy((window_t*) event.w);
					break;
				default:
					break;
				}
			} else {
				desktop_Input(&event);
			}
		}
		if (topWidget) {
			widget_draw(topWidget, COORDS(0, 0));
		}
	}
}

uint8_t gui_Init(void) {
	/* initialize event queue */
	eventQueue = xQueueCreate(10, sizeof(GUIEvent_t));

	if(!eventQueue) {
		return 0;
	}

	/* create GUI thread */
	if(xTaskCreate((TaskFunction_t )guiThread, "GUI", 300, NULL, 3, NULL)!=pdPASS) {
		return 0;
	}
	return 1;
}

void gui_SendEvent(GUIEvent_t *ev) {
	if(!eventQueue || !ev) {
		/* some pointer error */
		return;
	}
	BaseType_t yield;
	xQueueSendFromISR(eventQueue, ev, &yield);
	if (yield) {
		taskYIELD();
	}
}
