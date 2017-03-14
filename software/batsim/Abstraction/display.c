#include "../Abstraction/display.h"

#define RST_HIGH()			(GPIOB->BSRR = GPIO_PIN_9)
#define RST_LOW()			(GPIOB->BSRR = GPIO_PIN_9<<16u)
#define CS_HIGH()			(GPIOB->BSRR = GPIO_PIN_8)
#define CS_LOW()			(GPIOB->BSRR = GPIO_PIN_8<<16u)
#define RD_HIGH()			(GPIOC->BSRR = GPIO_PIN_9)
#define RD_LOW()			(GPIOC->BSRR = GPIO_PIN_9<<16u)
#define WR_HIGH()			(GPIOC->BSRR = GPIO_PIN_8)
#define WR_LOW()			(GPIOC->BSRR = GPIO_PIN_8<<16u)
#define RS_HIGH()			(GPIOC->BSRR = GPIO_PIN_7)
#define RS_LOW()			(GPIOC->BSRR = GPIO_PIN_7<<16u)

inline void setData(uint16_t data) {
	GPIOD->ODR = ((data & 0x00ff) << 8) + ((data & 0xff00) >> 8);
}

inline void selectRegister(uint8_t reg) {
	RS_LOW();
	setData(reg);
	WR_LOW();
	WR_HIGH();
}

inline void writeData(uint16_t data) {
	RS_HIGH();
	setData(data);
	WR_LOW();
	WR_HIGH();
}

void writeRegister(uint8_t reg, uint16_t data) {
	selectRegister(reg);
	writeData(data);
}

void SSD1289_Init(void) {
	/* enable oscillator */
	writeRegister(0x00,0x0001);
	/* Step-up cycle 8 color = fosc/4, step-up factor = +5/-4, step-up cycle 262k color = fosc/4, Op-amp power medium to large */
	writeRegister(0x03,0xA8A4);
	/* VCIX2 voltage = 5.1V */
	writeRegister(0x0C,0x0000);
	/* VLCD63 = Vref * 2.5 */
	writeRegister(0x0D,0x080C);
	/* VCOM = VLCD63 * 0.93 */
	writeRegister(0x0E,0x2B00);
	/* VCOMH = VLCD63 * 0.9 */
	writeRegister(0x1E,0x00B7);
	/* REV = 1, CAD = 0, BGR = 1, TB = 1, 319 lines */
	writeRegister(0x01,0x2B3F);
	/* B/C = 1, EOR = 1 */
	writeRegister(0x02,0x0600);
	/* disable sleep mode */
	writeRegister(0x10,0x0000);
	/* 65k color mode, automatic increase of address counter */
	writeRegister(0x11,0x6070);
	/* clear compare registers */
	writeRegister(0x05,0x0000);
	writeRegister(0x06,0x0000);
	/* pixels per line = 240, front porch = 30 */
	/* TODO this is the POR value and the interface isn't used anyway -> delete? */
	writeRegister(0x16,0xEF1C);
	/* VFP = 0, VBP = 3 */
	/* TODO interface isn't used, delete? */
	writeRegister(0x17,0x0003);

	writeRegister(0x07,0x0233);
	writeRegister(0x0B,0x0000);
	writeRegister(0x0F,0x0000);
	writeRegister(0x41,0x0000);
	writeRegister(0x42,0x0000);
	writeRegister(0x48,0x0000);
	writeRegister(0x49,0x013F);
	writeRegister(0x4A,0x0000);
	writeRegister(0x4B,0x0000);
	writeRegister(0x44,0xEF00);
	writeRegister(0x45,0x0000);
	writeRegister(0x46,0x013F);
	writeRegister(0x30,0x0707);
	writeRegister(0x31,0x0204);
	writeRegister(0x32,0x0204);
	writeRegister(0x33,0x0502);
	writeRegister(0x34,0x0507);
	writeRegister(0x35,0x0204);
	writeRegister(0x36,0x0204);
	writeRegister(0x37,0x0502);
	writeRegister(0x3A,0x0302);
	writeRegister(0x3B,0x0302);
	writeRegister(0x23,0x0000);
	writeRegister(0x24,0x0000);
	writeRegister(0x25,0x8000);
	writeRegister(0x4f,0x0000);
	writeRegister(0x4e,0x0000);
	selectRegister(0x22);
}
