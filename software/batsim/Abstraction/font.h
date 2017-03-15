#ifndef GUI_FONT_H_
#define GUI_FONT_H_

#include <stdint.h>

typedef struct {
	uint8_t *data;
    uint8_t width;
    uint8_t height;
} font_t;

extern font_t Font_Small;
extern font_t Font_Medium;
extern font_t Font_Big;

#endif
