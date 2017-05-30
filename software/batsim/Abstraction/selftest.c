#include "selftest.h"

#include "adc.h"

/* Maximum allowed deviation of voltages in percent that still passes the test */
#define VOLTAGE_MAX_DEV		10

typedef enum {TEST_PASSED = 0, TEST_FAILED} TestResult_t;

#define TEST_RESULT(exp, meas)	abs(((int16_t)(exp) - meas)*100/exp) <= VOLTAGE_MAX_DEV ? TEST_PASSED : TEST_FAILED

static uint16_t get_5VRail(void) {
	uint16_t result[2];
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) result, 2);
	/* wait for ADC to finish TODO this could be done nicer */
	HAL_Delay(10);
	/* convert to mV, full scale ADC is 6.6V */
	return (uint32_t) result[0] * 6600 / 4096;
}

static uint16_t get_3V3Rail(void) {
	uint16_t result[2];
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) result, 2);
	/* wait for ADC to finish TODO this could be done nicer */
	HAL_Delay(10);
	/* convert to mV, full scale ADC is 1.2V -> 1200mV * 4096 = 4915200 */
	return 4915200UL / result[1];
}

static void display_TestResult(const char *name, const char *result,
		TestResult_t res) {
	static uint8_t line = 1;
	display_SetForeground(COLOR_WHITE);
	display_String(0, line * 16, name);
	if (res == TEST_PASSED) {
		display_SetForeground(COLOR_GREEN);
		printf("%s%s PASSED\r\n", name, result);
	} else if (res == TEST_FAILED) {
		display_SetForeground(COLOR_RED);
		printf("%s%s FAILED\r\n", name, result);
	}
	display_String(160, line * 16, result);
	line++;
}

uint8_t selftest_Run(void) {
	display_SetBackground(COLOR_BLACK);
	display_SetForeground(COLOR_WHITE);
	display_SetFont(Font_Big);
	display_Clear();
	display_String(0, 0, "Running selftest...");
	printf("Running selftest...");

	char buffer[32];
	TestResult_t res;
	TestResult_t overallRes = TEST_PASSED;
	int32_t meas;

	/* Check frontpanel board voltages */
	meas = get_3V3Rail();
	res = TEST_RESULT(3300, meas);
	overallRes |= res;
	common_StringFromValue(buffer, 4, meas * 1000, &Unit_Voltage);
	display_TestResult("3.3V rail:", buffer, res);

	meas = get_5VRail();
	res = TEST_RESULT(5000, meas);
	overallRes |= res;
	common_StringFromValue(buffer, 4, meas * 1000, &Unit_Voltage);
	display_TestResult("5V rail:", buffer, res);

	// TODO check batsim board voltages and output stage

	return overallRes;
}
