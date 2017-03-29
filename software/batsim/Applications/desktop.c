#include "desktop.h"

AppInfo_t AppList[DESKTOP_MAX_APPS];
uint8_t NumApps;
uint8_t focussed = 0xff;

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
	focussed = num;
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
	if (focussed == num) {
		/* choose next running app */
		uint8_t i;
		for (i = 0; i < NumApps; i++) {
			if (AppList[i].state == APP_RUNNING) {
				/* found an active app */
				topWidget = AppList[i].topWidget;
				focussed = i;
				widget_RequestRedrawFull(topWidget);
				break;
			}
		}
		if (i >= NumApps) {
			/* no app active */
			topWidget = NULL;
			focussed = 0xff;
			/* clear app area */
			display_SetForeground(COLOR_BLACK);
			display_RectangleFull(DESKTOP_ICONBAR_WIDTH, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
		}
	}
	AppList[num].topWidget = NULL;
	desktop_Draw();
}

void desktop_Draw(void) {
	/* clear desktop area */
	display_SetForeground(COLOR_BLACK);
	display_RectangleFull(0, 0, DESKTOP_ICONBAR_WIDTH - 1, DISPLAY_HEIGHT - 1);
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
		if (focussed == i) {
			/* this is the active app */
			display_SetForeground(COLOR_BG_DEFAULT);
		} else {
			display_SetForeground(COLOR_BLACK);
		}
		display_VerticalLine(0, i * DESKTOP_ICONSPACING_Y + 3,
		DESKTOP_ICONSPACING_Y - 6);
		display_VerticalLine(1, i * DESKTOP_ICONSPACING_Y + 1,
		DESKTOP_ICONSPACING_Y - 2);
		display_HorizontalLine(3, i * DESKTOP_ICONSPACING_Y,
		DESKTOP_ICONBAR_WIDTH - 3);
		display_HorizontalLine(2, i * DESKTOP_ICONSPACING_Y + 1,
		DESKTOP_ICONBAR_WIDTH - 2);
		display_HorizontalLine(3, (i + 1) * DESKTOP_ICONSPACING_Y - 1,
		DESKTOP_ICONBAR_WIDTH - 3);
		display_HorizontalLine(2, (i + 1) * DESKTOP_ICONSPACING_Y - 2,
		DESKTOP_ICONBAR_WIDTH - 2);
	}
}

static uint8_t iAppToClose = 0;

static void msgBoxResult(MsgResult_t res) {
	if (res == MSG_RESULT_OK) {
		/* Stop app */
		xTaskNotify(AppList[iAppToClose].handle, SIGNAL_TERMINATE,
				eSetValueWithOverwrite);
		AppList[iAppToClose].state = APP_KILLSEND;
	}
}

void desktop_Input(GUIEvent_t *ev) {
	switch (ev->type) {
	case EVENT_TOUCH_PRESSED:
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
				if (focussed != num) {
					/* bring app into focus */
					topWidget = AppList[num].topWidget;
					focussed = num;
					widget_RequestRedrawFull(topWidget);
					// TODO only draw "active app lines"
					desktop_Draw();
				}
				break;
			case APP_STARTSEND:
			case APP_KILLSEND:
				/* do nothing */
				break;
			}
		}
		break;
	case EVENT_TOUCH_HELD:
		/* get icon number */
		if (ev->pos.x < DESKTOP_ICONBAR_WIDTH
				&& ev->pos.y < DESKTOP_ICONSPACING_Y * NumApps) {
			/* position is a valid icon */
			uint8_t num = ev->pos.y / DESKTOP_ICONSPACING_Y;
			switch(AppList[num].state) {
			case APP_RUNNING:
				iAppToClose = num;
				dialog_MessageBox("Close?", "Close this app?", MSG_ABORT_OK, msgBoxResult);
				break;
			case APP_STARTSEND:
			case APP_KILLSEND:
			case APP_STOPPED:
				/* do nothing */
				break;
			}
		}
		break;
	default:
		break;
	}
}
