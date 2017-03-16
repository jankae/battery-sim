#include "gui.h"

QueueHandle_t eventQueue = NULL;
widget_t *topWidget;
int32_t bla;

static void gui_createWindow(void) {

	dialog_InputValue("New value:", &bla, 0, 3000, &Unit_Current);
}

static void guiThread(void) {
	display_SetForeground(COLOR(0, 0, 0));
	display_SetBackground(COLOR(214, 211, 206));
	display_SetFont(Font_Big);

	GUIEvent_t event;
	button_t *b1 = button_new("Window", Font_Big, 0, gui_createWindow);
	button_t *b2 = button_new("Test2", Font_Big, 0, NULL);
	button_t *b3 = button_new("Test3", Font_Big, 0, NULL);

	container_t *c = container_new(SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT));
	container_attach(c, b1, COORDS(20, 0));
	container_attach(c, b2, COORDS(20, 50));
	container_attach(c, b3, COORDS(20, 100));
//	container_attach(c, b4, COORDS(20, 150));
//	container_attach(c, b5, COORDS(20, 200));
//	container_attach(c, b6, COORDS(20, 250));
	topWidget = c;
	display_Clear();
	widget_draw(topWidget, COORDS(0,0));
	usb_DisplayFlush();
	while (1) {
		if (xQueueReceive(eventQueue, &event, portMAX_DELAY)) {
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
								< topWidget->position.y + topWidget->size.y) {
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
			widget_draw(topWidget, COORDS(0, 0));
			usb_DisplayFlush();
		}
	}
}

void gui_Init(void){
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
