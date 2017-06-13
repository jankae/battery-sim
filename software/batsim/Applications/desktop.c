#include "desktop.h"

#include "buttons.h"
#include "app.h"

AppInfo_t AppList[DESKTOP_MAX_APPS];
uint8_t NumApps;
uint8_t focussed = 0xff;
uint8_t selected = 0;

extern widget_t *topWidget;
extern widget_t *selectedWidget;

void desktop_AddApp(AppInfo_t app) {
	if (NumApps < DESKTOP_MAX_APPS) {
		AppList[NumApps] = app;

		/* TaskHandle will be known only at startup */
		AppList[NumApps].handle = NULL;
		AppList[NumApps].topWidget = NULL;
		/* App is not started */
		AppList[NumApps].state = APP_STOPPED;

		/* one more app */
		NumApps++;
	}
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

static uint8_t AppNumFromTaskHandle(TaskHandle_t handle) {
	uint8_t num = 0;
	while (num < NumApps) {
		if (AppList[num].handle == handle)
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

void desktop_AppStopped(){
	uint8_t num = AppNumFromTaskHandle(xTaskGetCurrentTaskHandle());
	if (num >= NumApps) {
		/* failed to find correct app */
		return;
	}
	AppList[num].handle = NULL;
	AppList[num].state = APP_STOPPED;
	/* Remove the widgets this app created */
	widget_delete(AppList[num].topWidget);
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
	if(!selectedWidget) {
		/* no widget is selected, main control is with desktop */
		display_SetForeground(COLOR_SELECTED);
		uint8_t i;
#define ARROW_SIZE		6
		for (i = 0; i < ARROW_SIZE; i++) {
			display_VerticalLine(i + 1,
					selected * DESKTOP_ICONSPACING_Y + DESKTOP_ICONSPACING_Y / 2
							- ARROW_SIZE + i, 2 * (ARROW_SIZE - i));
		}
	}
	if (!topWidget) {
		/* Add app names in app area */
		display_SetBackground(COLOR_BLACK);
		display_SetFont(Font_Big);
		for (i = 0; i < NumApps; i++) {
			if(i==selected) {
				display_SetForeground(COLOR_SELECTED);
			} else {
				display_SetForeground(COLOR_WHITE);
			}
			display_String(DESKTOP_ICONBAR_WIDTH + 10,
					i * DESKTOP_ICONSPACING_Y
							+ (DESKTOP_ICONSPACING_Y - Font_Big.height) / 2,
					AppList[i].name);
		}
	}
}

static uint8_t iAppToClose = 0;

static void msgBoxResult(DialogResult_t res) {
	if (res == DIALOG_RESULT_OK) {
		/* Stop app */
		xTaskNotify(AppList[iAppToClose].handle, SIGNAL_TERMINATE,
				eSetBits);
		AppList[iAppToClose].state = APP_KILLSEND;
	}
}

static void desktop_SwitchToApp(uint8_t app) {
	switch(AppList[app].state) {
	case APP_STOPPED:
		/* start app */
		AppList[app].state = APP_STARTSEND;
		AppList[app].start();
		break;
	case APP_RUNNING:
		if (focussed != app) {
			/* bring app into focus */
			topWidget = AppList[app].topWidget;
			focussed = app;
			widget_RequestRedrawFull(topWidget);
			desktop_Draw();
		}
		break;
	case APP_STARTSEND:
	case APP_KILLSEND:
		/* do nothing */
		break;
	}
}

static void desktop_ConfirmClose(uint8_t app) {
	switch(AppList[app].state) {
	case APP_RUNNING:
		iAppToClose = app;
		dialog_MessageBox("Close?", Font_Big, "Close this app?", MSG_ABORT_OK, msgBoxResult, 0);
		break;
	case APP_STARTSEND:
	case APP_KILLSEND:
	case APP_STOPPED:
		/* do nothing */
		break;
	}
}

void desktop_Input(GUIEvent_t *ev) {
	if(selectedWidget) {
		/* de-select widget */
		widget_Select(NULL);
	}
	switch (ev->type) {
	case EVENT_TOUCH_PRESSED:
		/* get icon number */
		if (ev->pos.x < DESKTOP_ICONBAR_WIDTH
				&& ev->pos.y < DESKTOP_ICONSPACING_Y * NumApps) {
			/* position is a valid icon */
			uint8_t num = ev->pos.y / DESKTOP_ICONSPACING_Y;
			desktop_SwitchToApp(num);
		}
		break;
	case EVENT_TOUCH_HELD:
		/* get icon number */
		if (ev->pos.x < DESKTOP_ICONBAR_WIDTH
				&& ev->pos.y < DESKTOP_ICONSPACING_Y * NumApps) {
			/* position is a valid icon */
			uint8_t num = ev->pos.y / DESKTOP_ICONSPACING_Y;
			desktop_ConfirmClose(num);
		}
		break;
	case EVENT_BUTTON_CLICKED:
		/* switch selected App or start/stop App */
		switch (ev->button) {
		case BUTTON_DOWN:
			if (selected < NumApps - 1) {
				/* move selection down */
				selected++;
				desktop_Draw();
			}
			break;
		case BUTTON_UP:
			if (selected > 0) {
				/* move selection down */
				selected--;
				desktop_Draw();
			}
			break;
		case BUTTON_ENTER:
		case BUTTON_ENCODER:
			/* start App */
			desktop_SwitchToApp(selected);
			break;
		case BUTTON_ESC:
			/* close App */
			desktop_ConfirmClose(selected);
			break;
		}
		break;
	case EVENT_ENCODER_MOVED: {
		/* switch selected App */
		uint8_t new = selected;
		if (ev->movement > 0) {
			/* moving up */
			if (selected > ev->movement) {
				new -= ev->movement;
			} else {
				new = 0;
			}
		} else {
			if (NumApps - selected - 1 > -ev->movement) {
				new -= ev->movement;
			} else {
				new = NumApps - 1;
			}
		}
		if (new != selected) {
			selected = new;
			desktop_Draw();
		}
	}
		break;
	default:
		break;
	}
}
