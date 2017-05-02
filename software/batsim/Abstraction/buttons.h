#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "gui.h"
#include "gpio.h"

/* Button definitions */
// TODO extend list
#define BUTTON_ENCODER		0x00000001
#define BUTTON_1			0x00000002
#define BUTTON_2			0x00000004
#define BUTTON_3			0x00000008
#define BUTTON_4			0x00000010
#define BUTTON_5			0x00000020
#define BUTTON_6			0x00000040
#define BUTTON_7			0x00000080
#define BUTTON_8			0x00000100
#define BUTTON_9			0x00000200
#define BUTTON_0			0x00000400
#define BUTTON_DOT			0x00000800
#define BUTTON_DEL			0x00001000

void buttons_Init();

void buttons_Update(void);

#endif
