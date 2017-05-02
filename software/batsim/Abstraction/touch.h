#ifndef TOUCH_H_
#define TOUCH_H_

#include "gpio.h"
#include "gui.h"
#include "display.h"

#define TOUCH_RESOLUTION_X		DISPLAY_WIDTH
#define TOUCH_RESOLUTION_Y		DISPLAY_HEIGHT

/**
 * \brief Sets the SPI pins to idle values
 */
void touch_Init(void);

/**
 * \brief Retrieves the coordinates of the touchscreen
 *
 * \param c Pointer to coordinates struct
 * \return 1 if screen is being touched, 0 if not, -1 on error
 */
int8_t touch_GetCoordinates(coords_t *c);

void touch_Calibrate(void);

#endif
