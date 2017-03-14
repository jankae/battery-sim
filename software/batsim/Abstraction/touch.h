#ifndef TOUCH_H_
#define TOUCH_H_

#include "gpio.h"

#define TOUCH_RESOLUTION_X		240
#define TOUCH_RESOLUTION_Y		360

typedef struct {
	uint16_t x;
	uint16_t y;
} touchCoord_t;

/**
 * \brief Sets the SPI pins to idle values
 */
void touch_Init(void);

/**
 * \brief Retrieves the coordinates of the touchscreen
 *
 * \param c Pointer to coordinates struct
 * \return 1 if screen is being touched, 0 otherwise
 */
uint8_t touch_GetCoordinates(touchCoord_t *c);

#endif
