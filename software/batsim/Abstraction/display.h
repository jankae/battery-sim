#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "gpio.h"

#define COLOR(r, g, b)		(uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3))

void display_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void display_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

#endif
