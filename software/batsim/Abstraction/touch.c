#include "../Abstraction/touch.h"

#define CS_LOW()			(GPIOB->BSRR = GPIO_PIN_7<<16u)
#define CS_HIGH()			(GPIOB->BSRR = GPIO_PIN_7)
//#define DOUT_LOW()			(GPIOB->BSRR = GPIO_PIN_5<<16u)
//#define DOUT_HIGH()			(GPIOB->BSRR = GPIO_PIN_5)
//#define SCK_LOW()			(GPIOB->BSRR = GPIO_PIN_3<<16u)
//#define SCK_HIGH()			(GPIOB->BSRR = GPIO_PIN_3)
//#define DIN()				(GPIOB->IDR & GPIO_PIN_4)
#define PENIRQ()			(!(GPIOB->IDR & GPIO_PIN_8))

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

static uint8_t calibrating = 0;

int32_t offsetX = 0, offsetY = 0;
float scaleX = (float) TOUCH_RESOLUTION_X / 4096;
float scaleY = (float) TOUCH_RESOLUTION_Y / 4096;

extern SPI_HandleTypeDef hspi3;

void touch_Init(void) {
	CS_HIGH();
}

static uint16_t ADS7843_Read(uint8_t control) {
	CS_LOW();
	HAL_Delay(1);
	/* highest bit in control must always be one */
	control |= 0x80;
	uint16_t read = 0;
	/* transmit control word */
	HAL_SPI_Transmit(&hspi3, &control, 1, 10);
	/* read ADC result */
	HAL_SPI_Receive(&hspi3, (uint8_t*) &read, 2, 10);
	/* shift and mask 12-bit result */
	read >>= 3;
	read &= 0x0FFF;
	CS_HIGH();
	return read;
}

uint8_t touch_GetCoordinates(coords_t *c) {
	if(calibrating) {
		/* don't report coordinates while calibrating */
		return 0;
	}
	if (PENIRQ()) {
		/* screen is being touched */
		uint32_t rawX = ADS7843_Read(
		CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		uint32_t rawY = ADS7843_Read(
		CHANNEL_Y | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		if (!PENIRQ()) {
			/* touch has been released during measurement */
			return 0;
		}
		/* convert to screen resolution */
		c->x = rawX * scaleX + offsetX;
		c->y = rawY * scaleY + offsetY;
		return 1;
	} else {
		return 0;
	}
}

void touch_Calibrate(void) {
	calibrating = 1;
	uint8_t done = 0;
	/* display first calibration cross */
	display_SetBackground(COLOR_WHITE);
	display_SetForeground(COLOR_BLACK);
	display_Clear();
	display_Line(0, 0, 40, 40);
	display_Line(40, 0, 0, 40);
	volatile coords_t p1;
	do {
		/* wait for touch to be pressed */
		while (!PENIRQ())
			;
		/* get raw data */
		p1.x = ADS7843_Read(CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		p1.y = ADS7843_Read(CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		if (p1.x <= 1024 && p1.y <= 682)
			done = 1;
	} while (!done);
	while (PENIRQ())
		;
	/* display second calibration cross */
	display_Clear();
	display_Line(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 41,
	DISPLAY_HEIGHT - 41);
	display_Line(DISPLAY_WIDTH - 41, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1,
	DISPLAY_HEIGHT - 41);
	volatile coords_t p2;
	done = 0;
	do {
		/* wait for touch to be pressed */
		while (!PENIRQ())
			;
		/* get raw data */
		p2.x = ADS7843_Read(CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		p2.y = ADS7843_Read(CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
		if (p2.x >= 3584 && p2.y >= 3413)
			done = 1;
	} while (!done);

	/* calculate new calibration values */
	/* calculate scale */
	scaleX = (float) (DISPLAY_WIDTH - 40) / (p2.x - p1.x);
	scaleY = (float) (DISPLAY_HEIGHT - 40) / (p2.y - p1.y);
	/* calculate offset */
	offsetX = 20 - p1.x * scaleX;
	offsetY = 20 - p1.y * scaleY;

	while(PENIRQ());

	calibrating = 0;
}
