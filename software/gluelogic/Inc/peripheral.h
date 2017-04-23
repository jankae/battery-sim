#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#include "stm32f0xx_hal.h"

//#define MCP4706
//#define MCP4716
#define MCP4726

#define MCP47X6_ADDRESS		0xC0

void Peripheral_Init(void);
void DAC8552_SetChannel(uint8_t dac, uint8_t channel, uint16_t value);
void MCP41HVX1_SetWiper(uint8_t wiper);
void MCP47X6_SetChannel(uint16_t value);
void MCP47X6_Init(void);

#endif
