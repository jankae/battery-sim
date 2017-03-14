/**
 * \file
 */
#ifndef EXTDAC_H_
#define EXTDAC_H_

#include "gpio.h"

#define EXTDAC_CHANNEL_A		0
#define EXTDAC_CHANNEL_B		1


/**
 * \brief Sets the SPI pins to idle values
 */
void extDAC_Init(void);

/**
 * \brief Transfers a value to the external DAC (MCP48x2)
 *
 * \param channel DAC channel, either EXTDAC_CHANNEL_A or EXTDAC_CHANNEL_B
 * \param value New value for the selected channel (0-4095)
 */
void extDAC_Set(uint8_t channel, uint16_t value);


#endif
