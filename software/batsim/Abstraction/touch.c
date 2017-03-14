#include "../Abstraction/touch.h"

#define CS_LOW()			(GPIOB->BSRR = GPIO_PIN_3)
#define CS_HIGH()			(GPIOB->BSRR = GPIO_PIN_3<<16u)
#define DOUT_LOW()			(GPIOC->BSRR = GPIO_PIN_12)
#define DOUT_HIGH()			(GPIOC->BSRR = GPIO_PIN_12<<16u)
#define SCK_LOW()			(GPIOC->BSRR = GPIO_PIN_10)
#define SCK_HIGH()			(GPIOC->BSRR = GPIO_PIN_10<<16u)
#define DIN()				(GPIOC->IDR & GPIO_PIN_11)
#define BUSY()				(!(GPIOB->IDR & GPIO_PIN_4))
#define PENIRQ()			(!(GPIOB->IDR & GPIO_PIN_5))

/* A2-A0 bits in control word */
#define CHANNEL_X			(0x10)
#define CHANNEL_Y			(0x50)
#define CHANNEL_3			(0x20)
#define CHANNEL_4			(0x60)

/* SER/DFR bit */
#define SINGLE_ENDED		(0x04)
#define DIFFERENTIAL		(0x00)

/* Resolution */
#define BITS8				(0x80)
#define BITS12				(0x00)

/* Power down mode */
#define PD_PENIRQ			(0x00)
#define PD_NOIRQ			(0x01)
#define PD_ALWAYS_ON		(0x03)

void touch_Init(void) {
	CS_HIGH();
}


static uint16_t ADS7843_Read(uint8_t control) {
	SCK_LOW();
	CS_LOW();
	/* highest bit in control must always be one */
	control |= 0x80;
	uint16_t read = 0;
	uint8_t i;
	/* transmit control word */
	for (i = 0; i < 8; i++) {
		if (control & 0x80) {
			DOUT_HIGH();
		} else {
			DOUT_LOW();
		}
		SCK_HIGH();
		SCK_LOW();
		control <<= 1;
	}
	/* read ADC result */
	for (i = 0; i < 16; i++) {
		read <<= 1;
		if (DIN()) {
			read |= 1;
		}
		SCK_HIGH();
		SCK_LOW();
	}
	/* shift and mask 12-bit result */
	read >>= 3;
	read &= 0x0FFF;
	CS_HIGH();
	return read;
}

uint8_t touch_GetCoordinates(touchCoord_t *c) {
	if (PENIRQ()) {
		/* screen is being touched */
		uint32_t rawX = ADS7843_Read(
		CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		uint32_t rawY = ADS7843_Read(
		CHANNEL_Y | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		/* convert to screen resolution */
		c->x = (rawX * TOUCH_RESOLUTION_X) / 4096;
		c->y = (rawY * TOUCH_RESOLUTION_Y) / 4096;
		return 1;
	} else {
		return 0;
	}
}
