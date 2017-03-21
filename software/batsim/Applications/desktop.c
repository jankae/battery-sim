#include "desktop.h"

AppInfo_t AppList[DESKTOP_MAX_APPS];
uint8_t NumApps;

extern widget_t *topWidget;

void desktop_AddApp(AppInfo_t app) {
	AppList[NumApps] = app;

	/* TaskHandle will be known only at startup */
	AppList[NumApps].handle = NULL;
	AppList[NumApps].topWidget = NULL;
	/* App is not started */
	AppList[NumApps].state = APP_STOPPED;

	/* one more app */
	NumApps++;
}

static uint8_t AppNumFromStartFunction(void (*start)(void)) {
	uint8_t num = 0;
	while (num < NumApps) {
		if (AppList[num].start == start)
			/* found right one */
			break;
		num++;
	}
	return num;
}

void desktop_AppStarted(void (*start)(void), widget_t *top) {
	uint8_t num = AppNumFromStartFunction(start);
	if (num >= NumApps) {
		/* failed to find correct app */
		return;
	}
	AppList[num].handle = xTaskGetCurrentTaskHandle();
	AppList[num].state = APP_RUNNING;
	AppList[num].topWidget = top;
	/* bring app into focus */
	topWidget = AppList[num].topWidget;
	widget_RequestRedrawFull(topWidget);
	desktop_Draw();
}

void desktop_AppStopped(void (*start)(void)){
	uint8_t num = AppNumFromStartFunction(start);
	if (num >= NumApps) {
		/* failed to find correct app */
		return;
	}
	AppList[num].handle = NULL;
	AppList[num].state = APP_STOPPED;
	/* change focus if this app had it */
	if (topWidget == AppList[num].topWidget) {
		/* choose next running app */
		uint8_t i;
		for (i = 0; i < NumApps; i++) {
			if (AppList[i].state == APP_RUNNING) {
				/* found an active app */
				topWidget = AppList[i].topWidget;
				widget_RequestRedrawFull(topWidget);
				break;
			}
		}
		if (i >= NumApps) {
			/* no app active */
			topWidget = NULL;
			/* clear app area */
			display_SetForeground(COLOR_BLACK);
			display_RectangleFull(DESKTOP_ICONBAR_WIDTH, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
		}
	}
	desktop_Draw();
}

void desktop_Draw(void) {
	uint8_t i;
	for (i = 0; i < NumApps; i++) {
		if (AppList[i].state == APP_RUNNING) {
			display_Image(DESKTOP_ICONOFFSET_X,
					i * DESKTOP_ICONSPACING_Y + DESKTOP_ICONOFFSET_Y,
					&AppList[i].icon);
		} else {
			display_ImageGrayscale(DESKTOP_ICONOFFSET_X,
					i * DESKTOP_ICONSPACING_Y + DESKTOP_ICONOFFSET_Y,
					&AppList[i].icon);
		}
	}
}

void desktop_Input(GUIEvent_t *ev) {
	switch (ev->type) {
	case EVENT_TOUCH_RELEASED:
		/* get icon number */
		if (ev->pos.x < DESKTOP_ICONBAR_WIDTH
				&& ev->pos.y < DESKTOP_ICONSPACING_Y * NumApps) {
			/* position is a valid icon */
			uint8_t num = ev->pos.y / DESKTOP_ICONSPACING_Y;
			switch(AppList[num].state) {
			case APP_STOPPED:
				/* start app */
				AppList[num].state = APP_STARTSEND;
				AppList[num].start();
				break;
			case APP_RUNNING:
				if (topWidget != AppList[num].topWidget) {
					/* bring app into focus */
					topWidget = AppList[num].topWidget;
					widget_RequestRedrawFull(topWidget);
				}
				break;
			case APP_STARTSEND:
			case APP_KILLSEND:
				/* do nothing */
				break;
			}
		}
		break;
	default:
		break;
	}
}
