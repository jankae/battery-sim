#include "display.h"

#define RST_HIGH()			(GPIOE->BSRR = GPIO_PIN_0)
#define RST_LOW()			(GPIOE->BSRR = GPIO_PIN_0<<16u)
#define CS_HIGH()			(GPIOB->BSRR = GPIO_PIN_9)
#define CS_LOW()			(GPIOB->BSRR = GPIO_PIN_9<<16u)
#define RD_HIGH()			(GPIOC->BSRR = GPIO_PIN_6)
#define RD_LOW()			(GPIOC->BSRR = GPIO_PIN_6<<16u)
#define WR_HIGH()			(GPIOB->BSRR = GPIO_PIN_15)
#define WR_LOW()			(GPIOB->BSRR = GPIO_PIN_15<<16u)
#define RS_HIGH()			(GPIOB->BSRR = GPIO_PIN_14)
#define RS_LOW()			(GPIOB->BSRR = GPIO_PIN_14<<16u)

color_t foreground;
color_t background;
font_t font;

inline void setData(uint16_t data) {
	GPIOD->ODR = ((data & 0x00ff) << 8) + ((data & 0xff00) >> 8);
}

inline void selectRegister(uint8_t reg) {
	RS_LOW();
	setData(reg);
//	HAL_Delay(1);
	CS_LOW();
//	HAL_Delay(1);
	WR_LOW();
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	WR_HIGH();
//	HAL_Delay(1);
	CS_HIGH();
//	HAL_Delay(1);
}

inline void writeData(uint16_t data) {
	RS_HIGH();
	setData(data);
//	HAL_Delay(1);
	CS_LOW();
//	HAL_Delay(1);
	WR_LOW();
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
	asm volatile("nop");
//	HAL_Delay(1);
	WR_HIGH();
//	HAL_Delay(1);
	CS_HIGH();
//	HAL_Delay(1);
}

uint16_t readData(void) {
	/* configure data pins as inputs */
	GPIO_InitTypeDef GPIO_InitStruct;
	/*Configure GPIO pins : PD8 PD9 PD10 PD11
	 PD12 PD13 PD14 PD15
	 PD0 PD1 PD2 PD3
	 PD4 PD5 PD6 PD7 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
			| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0
			| GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
			| GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Read cycle */
	RS_HIGH();
	HAL_Delay(2);
	CS_LOW();
	HAL_Delay(2);
	RD_LOW();
	HAL_Delay(2);
	uint16_t data = GPIOD->IDR;
	CS_HIGH();
	HAL_Delay(2);
	RD_HIGH();
	HAL_Delay(2);

	/* configure data pins as outputs */
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	return data;
}

void writeRegister(uint8_t reg, uint16_t data) {
	selectRegister(reg);
	writeData(data);
}

uint16_t readRegister(uint8_t reg) {
	selectRegister(reg);
	return readData();
}

inline void setYStartStop(uint16_t start, uint16_t stop) {
	writeRegister(0x44, (stop << 8) + start);
}

inline void setXStart(uint16_t start) {
	writeRegister(0x45, start);
}

inline void setXStop(uint16_t stop) {
	writeRegister(0x46, stop);
}

void setXY(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	/* convert to landscape mode */
	y0 = DISPLAY_HEIGHT - y0 - 1;
	y1 = DISPLAY_HEIGHT - y1 - 1;
	/* set start and stop values */
	setYStartStop(y1, y0);
	setXStart(x0);
	setXStop(x1);
	/* start in top left corner */
	writeRegister(0x4e, y0);
	writeRegister(0x4f, x0);
	selectRegister(0x22);
}

void SSD1289_Init(void) {
	RST_HIGH();
	HAL_Delay(5);
	RST_LOW();
	HAL_Delay(15);
	RST_HIGH();
	HAL_Delay(15);
	CS_LOW();
	RD_HIGH();
	WR_HIGH();



	/* enable oscillator */
	writeRegister(0x00, 0x0001);
	/* Step-up cycle 8 color = fosc/4, step-up factor = +5/-4, step-up cycle 262k color = fosc/4, Op-amp power medium to large */
	writeRegister(0x03, 0xA8A4);
	/* VCIX2 voltage = 5.1V */
	writeRegister(0x0C, 0x0000);
	/* VLCD63 = Vref * 2.5 */
	writeRegister(0x0D, 0x080C);
	/* VCOM = VLCD63 * 0.93 */
	writeRegister(0x0E, 0x2B00);
	/* VCOMH = VLCD63 * 0.9 */
	writeRegister(0x1E, 0x00B7);
	/* REV = 1, CAD = 0, BGR = 1, TB = 1, 319 lines */
	writeRegister(0x01, 0x2B3F);
	/* B/C = 1, EOR = 1 */
	writeRegister(0x02, 0x0600);
	/* disable sleep mode */
	writeRegister(0x10, 0x0000);
	/* 65k color mode, automatic increase of address counter */
	writeRegister(0x11, 0x6068);
	/* clear compare registers */
	writeRegister(0x05, 0x0000);
	writeRegister(0x06, 0x0000);
	/* pixels per line = 240, front porch = 30 */
	/* TODO this is the POR value and the interface isn't used anyway -> delete? */
	writeRegister(0x16, 0xEF1C);
	/* VFP = 0, VBP = 3 */
	/* TODO interface isn't used, delete? */
	writeRegister(0x17, 0x0003);

	writeRegister(0x07, 0x0233);
	writeRegister(0x0B, 0x0000);
	writeRegister(0x0F, 0x0000);
	writeRegister(0x41, 0x0000);
	writeRegister(0x42, 0x0000);
	writeRegister(0x48, 0x0000);
	writeRegister(0x49, 0x013F);
	writeRegister(0x4A, 0x0000);
	writeRegister(0x4B, 0x0000);
	writeRegister(0x44, 0xEF00);
	writeRegister(0x45, 0x0000);
	writeRegister(0x46, 0x013F);
	writeRegister(0x30, 0x0707);
	writeRegister(0x31, 0x0204);
	writeRegister(0x32, 0x0204);
	writeRegister(0x33, 0x0502);
	writeRegister(0x34, 0x0507);
	writeRegister(0x35, 0x0204);
	writeRegister(0x36, 0x0204);
	writeRegister(0x37, 0x0502);
	writeRegister(0x3A, 0x0302);
	writeRegister(0x3B, 0x0302);
	writeRegister(0x23, 0x0000);
	writeRegister(0x24, 0x0000);
	writeRegister(0x25, 0x8000);
	writeRegister(0x4f, 0x0000);
	writeRegister(0x4e, 0x0000);

	selectRegister(0x22);
}

void display_Init(void){
	SSD1289_Init();
	background = COLOR_BG_DEFAULT;
	foreground = COLOR_FG_DEFAULT;
	font = Font_Big;
}

inline void display_SetFont(font_t f) {
	font = f;
}

inline void display_SetForeground(color_t c) {
	foreground = c;
}

inline color_t display_GetForeground(void) {
	return foreground;
}

inline void display_SetBackground(color_t c) {
	background = c;
}

inline color_t display_GetBackground(void) {
	return background;
}

void display_Clear() {
//	usb_DisplayCommand(0, 0);
//	usb_DisplayCommand(1, 0);
//	usb_DisplayCommand(2, DISPLAY_WIDTH - 1);
//	usb_DisplayCommand(3, DISPLAY_HEIGHT - 1);
	setXY(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
	uint32_t i = DISPLAY_WIDTH * DISPLAY_HEIGHT;
	for (; i > 0; i--) {
		writeData(background);
//		usb_DisplayCommand(4, background);
	}
}

void display_Pixel(uint16_t x, uint16_t y, uint16_t color) {
//	usb_DisplayCommand(0, x);
//	usb_DisplayCommand(1, y);
//	usb_DisplayCommand(4, color);
	setXY(x, y, x, y);
	writeData(color);
}

void display_HorizontalLine(uint16_t x, uint16_t y, uint16_t length) {
//	usb_DisplayCommand(0, x);
//	usb_DisplayCommand(1, y);
//	usb_DisplayCommand(2, x + length - 1);
//	usb_DisplayCommand(3, y);
	setXY(x, y, x + length - 1, y);
	for (; length > 0; length--) {
		writeData(foreground);
//		usb_DisplayCommand(4, foreground);
	}
}

void display_VerticalLine(uint16_t x, uint16_t y, uint16_t length) {
//	usb_DisplayCommand(0, x);
//	usb_DisplayCommand(1, y);
//	usb_DisplayCommand(2, x);
//	usb_DisplayCommand(3, y + length - 1);
	setXY(x, y, x, y + length - 1);
	for (; length > 0; length--) {
		writeData(foreground);
//		usb_DisplayCommand(4, foreground);
	}
}

void display_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	uint16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	uint16_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int16_t err = (dx > dy ? dx : -dy) / 2, e2;

	for (;;) {
		display_Pixel(x0, y0, foreground);
		if (x0 == x1 && y0 == y1)
			break;
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void display_Rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	display_VerticalLine(x0, y0, y1 - y0);
	display_VerticalLine(x1, y0 + 1, y1 - y0);
	display_HorizontalLine(x0 + 1, y0, x1 - x0);
	display_HorizontalLine(x0, y1, x1 - x0);
}

void display_RectangleFull(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
//	usb_DisplayCommand(0, x0);
//	usb_DisplayCommand(1, y0);
//	usb_DisplayCommand(2, x1);
//	usb_DisplayCommand(3, y1);
	setXY(x0, y0, x1, y1);
	uint32_t i = (x1 - x0 + 1) * (y1 - y0 + 1);
	for (; i > 0; i--) {
		writeData(foreground);
//		usb_DisplayCommand(4, foreground);
	}
}

void display_Char(uint16_t x, uint16_t y, uint8_t c) {
//	usb_DisplayCommand(0, x);
//	usb_DisplayCommand(1, y);
//	usb_DisplayCommand(2, x + font.width - 1);
//	usb_DisplayCommand(3, y + font.height - 1);
	setXY(x, y, x + font.width - 1, y + font.height - 1);
	/* number of bytes in font per row */
	uint8_t yInc = (font.width - 1) / 8 + 1;
	const uint8_t *charIndex = font.data + c * yInc * font.height;
	uint8_t i, j;
	uint8_t startMask = 0x80 >> (yInc * 8 - font.width);
	for (i = 0; i < font.height; i++) {
		uint8_t offset = (i + 1) * yInc - 1;
		uint8_t bitMask = startMask;
		for (j = 0; j < font.width; j++) {
			uint16_t color;
			if (charIndex[offset] & bitMask) {
				color = foreground;
			} else {
				color = background;
			}
			writeData(color);
//			usb_DisplayCommand(4, color);
			bitMask >>= 1;
			if (!bitMask) {
				bitMask = 0x80;
				offset--;
			}
		}
	}
}

void display_String(uint16_t x, uint16_t y, const char *s) {
	while (*s) {
		display_Char(x, y, *s++);
		x += font.width;
		if (x > DISPLAY_WIDTH - font.width)
			break;
	}
}

void display_Image(uint16_t x, uint16_t y, const Image_t *im) {
//	usb_DisplayCommand(0, x);
//	usb_DisplayCommand(1, y);
//	usb_DisplayCommand(2, x + im->width - 1);
//	usb_DisplayCommand(3, y + im->height - 1);
	setXY(x, y, x + im->width - 1, y + im->height - 1);
	uint32_t i = im->width * im->height;
	const uint16_t *ptr = im->data;
	for (; i > 0; i--) {
		writeData(*ptr++);
//		usb_DisplayCommand(4, *ptr++);
	}
}
void display_ImageGrayscale(uint16_t x, uint16_t y, const Image_t *im){
	//	usb_DisplayCommand(0, x);
	//	usb_DisplayCommand(1, y);
	//	usb_DisplayCommand(2, x + im->width - 1);
	//	usb_DisplayCommand(3, y + im->height - 1);
	setXY(x, y, x + im->width - 1, y + im->height - 1);
	uint32_t i = im->width * im->height;
	const uint16_t *ptr = im->data;
	for (; i > 0; i--) {
		/* convert to grayscale */
		uint16_t gray = COLOR_R(*ptr) + COLOR_G(*ptr) + COLOR_B(*ptr);
		gray /= 3;
		writeData(COLOR(gray, gray, gray));
//		usb_DisplayCommand(4, COLOR(gray, gray, gray));
		ptr++;
	}
}
