#include "gui.h"

QueueHandle_t eventQueue = NULL;
widget_t *topWidget;

static void guiThread(void) {

	topWidget = NULL;
	GUIEvent_t event;

	desktop_Draw();

	while (1) {
		if (xQueueReceive(eventQueue, &event, 100)) {
			if (topWidget) {
				switch (event.type) {
				case EVENT_TOUCH_PRESSED:
				case EVENT_TOUCH_RELEASED:
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
					}
					break;
				case EVENT_WINDOW_CLOSE:
					window_destroy((window_t*) event.w);
					break;
				default:
					break;
				}
			}
			desktop_Input(&event);
		}
		if (topWidget) {
			widget_draw(topWidget, COORDS(0, 0));
		}
		usb_DisplayFlush();
	}
}

void gui_Init(void) {
	/* initialize event queue */
	eventQueue = xQueueCreate(10, sizeof(GUIEvent_t));

	/* create GUI thread */
	xTaskCreate(guiThread, "GUI", 300, NULL, 3, NULL);
}

void gui_SendEvent(GUIEvent_t *ev) {
	BaseType_t yield;
	xQueueSendFromISR(eventQueue, ev, &yield);
	if (yield) {
		taskYIELD();
	}
}
