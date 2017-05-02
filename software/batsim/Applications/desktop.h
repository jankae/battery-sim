#ifndef DESKTOP_H_
#define DESKTOP_H_


#include "definitions.h"
#include "gui.h"
#include "task.h"

#define DESKTOP_MAX_APPS		5

#define DESKTOP_MAX_NAMELENGTH	20

#define DESKTOP_ICONBAR_WIDTH	40
#define DESKTOP_ICONSPACING_Y	40
#define DESKTOP_ICONOFFSET_Y	4
#define DESKTOP_ICONOFFSET_X	4

typedef enum {
	APP_STOPPED, APP_STARTSEND, APP_RUNNING, APP_KILLSEND
} AppState_t;

typedef struct {
	char name[DESKTOP_MAX_NAMELENGTH + 1];
	TaskHandle_t handle;
	void (*start)(void);
	widget_t *topWidget;
	AppState_t state;
	Image_t icon;
} AppInfo_t;

void desktop_AddApp(AppInfo_t app);
void desktop_AppStarted(void (*start)(void), widget_t *top);
void desktop_AppStopped(void (*start)(void));
void desktop_Draw(void);
void desktop_Input(GUIEvent_t *ev);

#endif
