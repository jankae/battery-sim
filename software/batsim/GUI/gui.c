#include "gui.h"

QueueHandle_t eventQueue = NULL;

static void guiThread(void) {
	display_SetForeground(COLOR(0, 0, 0));
	display_SetBackground(COLOR(214, 211, 206));
	display_SetFont(Font_Big);

	GUIEvent_t event;
	button_t b1;
	button_create(&b1, "Test1", Font_Big, 0, NULL);
	button_t b2;
	button_create(&b2, "Test2", Font_Big, 0, NULL);
	button_t b3;
	button_create(&b3, "Test3", Font_Big, 0, NULL);
	button_t b4;
	button_create(&b4, "Test4", Font_Big, 0, NULL);
	button_t b5;
	button_create(&b5, "Test5", Font_Big, 0, NULL);
	button_t b6;
	button_create(&b6, "Test6", Font_Big, 0, NULL);
	coords_t coord = { .x = 0, .y = 0 };
	container_t c;
	container_create(&c, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	container_attach(&c, &b1, 20, 0);
	container_attach(&c, &b2, 20, 50);
	container_attach(&c, &b3, 20, 100);
	container_attach(&c, &b4, 20, 150);
	container_attach(&c, &b5, 20, 200);
	container_attach(&c, &b6, 20, 250);
	display_Clear();
	widget_draw(&c, coord);
	usb_DisplayFlush();
	while (1) {
		if (xQueueReceive(eventQueue, &event, portMAX_DELAY)) {
			/* got a new event */
			if (event.type == GUI_TOUCH_PRESSED
					|| event.type == GUI_TOUCH_RELEASED) {
				/* send event to root widget */
				widget_input(&c, &event);
				widget_draw(&c, coord);
				usb_DisplayFlush();
			}
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
