#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
} coords_t;

#define COORDS(v1, v2)	((coords_t){.x=v1, .y=v2})
#define SIZE(v1, v2)	COORDS(v1, v2)

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
        int32_t scaleToLow, int32_t scaleToHigh);

#endif
