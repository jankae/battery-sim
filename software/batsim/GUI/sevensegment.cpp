#include "sevensegment.h"

/* Index			Symbol
 * 0-9				0-9
 * 10				Negative-sign
 * 11				Blank
 */
static const uint8_t digitToSegments[12] = {
		0b00111111,
		0b00000110,
		0b01011011,
		0b01001111,
		0b01100110,
		0b01101101,
		0b01111101,
		0b00000111,
		0b01111111,
		0b01101111,
		0b01000000,
		0b00000000,
};

/* 0 = vertical, 1 = horizontal */
static const uint8_t segmentOrientation = 0b01001001;
static const uint8_t segmentStartX[7] = {
		0, 1, 1, 0, 0, 0, 0
};
static const uint8_t segmentStartY[7] = {
		0, 0, 1, 2, 1, 0, 1
};

sevensegment_t* sevensegment_new(int32_t *value, uint8_t sLength,
		uint8_t sWidth, uint8_t length, uint8_t dot, color_t color) {
	sevensegment_t* s = (sevensegment_t*) pvPortMalloc(sizeof(sevensegment_t));
	if (!s) {
		/* malloc failed */
		return NULL;
	}
	widget_init((widget_t*) s);
	/* set widget functions */
	s->base.func.draw = sevensegment_draw;
	s->base.func.input = sevensegment_input;
	/* set member variables */
	s->value = value;
	s->segmentLength = sLength;
	s->segmentWidth = sWidth;
	s->length = length;
	s->dot = dot;
	s->color = color;

	uint16_t height = sWidth + 2 * sLength;
	uint16_t digitWidth = sWidth + sLength;
	s->base.size.y = height;
	s->base.size.x = digitWidth * length + sWidth * (length - 1);

	return s;
}

static void draw_Digit(sevensegment_t *s, int16_t x, int16_t y, uint8_t digit) {
	uint8_t i;
	for (i = 0; i < 7; i++) {
		/* Select color for this segment */
		if ((1 << i) & digitToSegments[digit]) {
			display_SetForeground(s->color);
		} else {
			display_SetForeground(SEVENSEGMENT_BG_COLOR);
		}

		/* draw the segment */
		int16_t offsetX = x + segmentStartX[i] * (s->segmentLength + 1);
		int16_t offsetY = y + segmentStartY[i] * (s->segmentLength + 1);

		if ((1 << i) & segmentOrientation) {
			/* this is a horizontal segment */
			display_HorizontalLine(offsetX + 1, offsetY, s->segmentLength);
			uint8_t j;
			for (j = 1; j <= s->segmentWidth / 2; j++) {
				display_HorizontalLine(offsetX + j + 1, offsetY + j,
						s->segmentLength - 2 * j);
				display_HorizontalLine(offsetX + j + 1, offsetY - j,
						s->segmentLength - 2 * j);
			}
		} else {
			/* this is a vertical segment */
			display_VerticalLine(offsetX, offsetY + 1, s->segmentLength);
			uint8_t j;
			for (j = 1; j <= s->segmentWidth / 2; j++) {
				display_VerticalLine(offsetX + j, offsetY + j + 1,
						s->segmentLength - 2 * j);
				display_VerticalLine(offsetX - j, offsetY + j + 1,
						s->segmentLength - 2 * j);
			}
		}
	}
}

void sevensegment_draw(widget_t *w, coords_t offset) {
	sevensegment_t * s = (sevensegment_t*) w;
	int32_t buf = *s->value;
	uint8_t neg = 0;
	if (buf < 0) {
		buf = -buf;
		neg = 1;
	}
	uint8_t i;
	int16_t x = offset.x
			+ (s->length - 1) * (s->segmentLength + 2 * s->segmentWidth)
			+ s->segmentWidth / 2;
	int16_t y = offset.y + s->segmentWidth / 2;
	for (i = 0; i < s->length; i++) {
		if (i == s->length - 1) {
			/* this is the negative sign position */
			if (neg) {
				draw_Digit(s, x, y, 10);
			} else {
				draw_Digit(s, x, y, 11);
			}
		} else {
			draw_Digit(s, x, y, buf % 10);
		}
		if (s->dot && (s->dot == i + 1)) {
			/* draw dot in front of current digit */
			int16_t offsetX = x - s->segmentWidth;
			int16_t offsetY = y + 2 * (s->segmentLength + 1)
					- s->segmentWidth / 2;
			display_SetForeground(s->color);
			display_VerticalLine(offsetX, offsetY + 1, s->segmentWidth);
			uint8_t j;
			for (j = 1; j <= s->segmentWidth / 2; j++) {
				display_VerticalLine(offsetX + j, offsetY + j + 1,
						s->segmentWidth - 2 * j);
				display_VerticalLine(offsetX - j, offsetY + j + 1,
						s->segmentWidth - 2 * j);
			}
		}
		x -= s->segmentLength + 2 * s->segmentWidth;
		buf /= 10;
	}
}

void sevensegment_input(widget_t *w, GUIEvent_t *ev) {
    /* sevensegment doesn't handle any input */
	return;
}
