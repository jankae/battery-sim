#include "../Abstraction/extDAC.h"

#define LDAC_HIGH()			(GPIOE->BSRR = GPIO_PIN_11)
#define LDAC_LOW()			(GPIOE->BSRR = GPIO_PIN_11<<16u)
#define SDI_HIGH()			(GPIOE->BSRR = GPIO_PIN_12)
#define SDI_LOW()			(GPIOE->BSRR = GPIO_PIN_12<<16u)
#define SCK_HIGH()			(GPIOE->BSRR = GPIO_PIN_13)
#define SCK_LOW()			(GPIOE->BSRR = GPIO_PIN_13<<16u)
#define CS_HIGH()			(GPIOE->BSRR = GPIO_PIN_14)
#define CS_LOW()			(GPIOE->BSRR = GPIO_PIN_14<<16u)

void extDAC_Init(void){
	CS_HIGH();
	SCK_LOW();
	LDAC_HIGH();
}

static void MCP48x2_SPI(uint16_t data) {
	CS_LOW();
	uint8_t i;
	for (i = 0; i < 16; i++) {
		if (data & 0x8000) {
			SDI_HIGH();
		} else {
			SDI_LOW();
		}
		SCK_HIGH();
		SCK_LOW();
		data <<= 1;
	}
	CS_HIGH();
}

void extDAC_Set(uint8_t channel, uint16_t value){
	/* use only the lower 12 bit of value */
	uint16_t word = value & 0x0FFF;
	/* Gain of 1, output enabled */
	word |= 0x3000;
	/* select channel */
	if(channel) {
		word |= 0x8000;
	}
	/* transfer to DAC */
	MCP48x2_SPI(word);
	/* update output */
	LDAC_LOW();
	LDAC_HIGH();
}
