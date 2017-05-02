#include "buttons.h"

#define BUTTONS_ROWS		6
#define BUTTONS_COLUMNS		4

/* GPIO definitions */
#define PORT_OUT_1			GPIOA
#define PIN_OUT_1			GPIO_PIN_8
#define PORT_OUT_2			GPIOC
#define PIN_OUT_2			GPIO_PIN_9
#define PORT_OUT_3			GPIOC
#define PIN_OUT_3			GPIO_PIN_11
#define PORT_OUT_4			GPIOC
#define PIN_OUT_4			GPIO_PIN_10
#define PORT_OUT_5			GPIOC
#define PIN_OUT_5			GPIO_PIN_7
#define PORT_OUT_6			GPIOB
#define PIN_OUT_6			GPIO_PIN_1

#define PORT_IN_1			GPIOA
#define PIN_IN_1			GPIO_PIN_15
#define PORT_IN_2			GPIOC
#define PIN_IN_2			GPIO_PIN_12
#define PORT_IN_3			GPIOC
#define PIN_IN_3			GPIO_PIN_14
#define PORT_IN_4			GPIOE
#define PIN_IN_4			GPIO_PIN_3

/* Matrix representation of the buttons */
const uint32_t matrix[BUTTONS_ROWS][BUTTONS_COLUMNS] = {
		{BUTTON_1,			BUTTON_2,			BUTTON_3,			BUTTON_ENCODER},
		{BUTTON_4,			BUTTON_5,			BUTTON_5,			0},
		{BUTTON_7,			BUTTON_8,			BUTTON_9,			0},
		{BUTTON_DOT,		BUTTON_0,			BUTTON_DEL,			0},
		{0,					0,					0,					0},
		{0,					0,					0,					0},
};

/* Current state of the buttons */
uint32_t state;
/* Active row of the button matrix */
uint8_t selectedRow;


/**
 * @brief Enables one row of the switch matrix
 * @param row Number of the row to activate (0-5)
 */
static inline void setRow(uint8_t row) {
	/* deactivate all columns */
	PORT_OUT_1->BSRR = PIN_OUT_1;
	PORT_OUT_2->BSRR = PIN_OUT_2;
	PORT_OUT_3->BSRR = PIN_OUT_3;
	PORT_OUT_4->BSRR = PIN_OUT_4;
	PORT_OUT_5->BSRR = PIN_OUT_5;
	PORT_OUT_6->BSRR = PIN_OUT_6;
	/* activate selected column */
	switch (row) {
	case 0:
		PORT_OUT_1->BRR = PIN_OUT_1;
		break;
	case 1:
		PORT_OUT_2->BRR = PIN_OUT_2;
		break;
	case 2:
		PORT_OUT_3->BRR = PIN_OUT_3;
		break;
	case 3:
		PORT_OUT_4->BRR = PIN_OUT_4;
		break;
	case 4:
		PORT_OUT_5->BRR = PIN_OUT_5;
		break;
	case 5:
		PORT_OUT_6->BRR = PIN_OUT_6;
		break;
	default:
		/* invalid row, do nothing */
		break;
	}
}

/**
 * @brief Retrieves the value of a column of the switch matrix
 * @param column Number of column (0-3)
 * @return 1, if button on this row is pressed, 0 otherwise
 */
static inline uint8_t getColumn(uint8_t column) {
	uint32_t res = 1;
	/* activate selected column */
	switch (column) {
	case 0:
		res = PORT_IN_1->IDR & PIN_IN_1;
		break;
	case 1:
		res = PORT_IN_2->IDR & PIN_IN_2;
		break;
	case 2:
		res = PORT_IN_3->IDR & PIN_IN_3;
		break;
	case 3:
		res = PORT_IN_4->IDR & PIN_IN_4;
		break;
	default:
		/* invalid column, do nothing */
		break;
	}
	return res ? 0 : 1;
}

/**
 * @brief Initializes button matrix
 */
void buttons_Init() {
	/* no button is pressed at the beginning */
	state = 0;
	selectedRow = 0;
	setRow(selectedRow);
}


/**
 * @brief Updates the switch matrix state, one row at a time.
 *
 * This function should be called from an interrupt e.g. every ms
 */
void buttons_Update(void) {
	/* evaluate columns for the active row */
	uint8_t i;
	/* save the old button state */
	uint32_t oldState = state;
	for (i = 0; i < BUTTONS_COLUMNS; i++) {
		if (getColumn(i)) {
			/* the button is pressed */
			state |= matrix[selectedRow][i];
		} else {
			/* the button is not pressed */
			state &= ~matrix[selectedRow][i];
		}
	}

	/* switch to next row */
	selectedRow++;
	if (selectedRow == BUTTONS_ROWS) {
		selectedRow = 0;
	}
	setRow(selectedRow);

	/* Check for changes in buttons */
	if (oldState != state) {
		/* something has happened */
		/* iterate over all buttons and send events */
		uint32_t button;
		for (button = 0x01; button; button <<= 1) {
			if ((oldState ^ state) & button) {
				/* this button has changed */
				if (state & button) {
					/* this button has been pressed */
					GUIEvent_t ev;
					ev.type = EVENT_BUTTON_CLICKED;
					ev.button = button;
					gui_SendEvent(&ev);
				} else {
					// TODO could a 'button release'-event be useful?
				}
			}
		}
	}
}
