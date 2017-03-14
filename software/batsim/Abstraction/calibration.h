#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include <stdint.h>
#include <string.h>

/* Defines for automatic calculation of default calibration values */
#define MAX_VOLTAGE_LOW_SCALE		5000000
#define MAX_VOLTAGE_HIGH_SCALE		22500000//20000000 1k5 instead of 12k||2k for now
#define MAX_CURRENT_LOW_ADC			303030
#define MAX_CURRENT_HIGH_ADC		3030303
#define MAX_PUSHPULL_OUT			21250000
#define MAX_BATTERY					21648936

#define DAC_MAX						4096
#define ADC_MAX_SINGLE				4096
#define ADC_MAX_DIFF				2048

typedef enum {
	CAL_VOLTAGE_DAC_LOW = 0,
	CAL_VOLTAGE_DAC_HIGH = 1,
	CAL_ADC_CURRENT_OFFSET_LOW = 2,
	CAL_ADC_CURRENT_OFFSET_HIGH = 3,
	CAL_ADC_CURRENT_FACTOR_LOW = 4,
	CAL_ADC_CURRENT_FACTOR_HIGH = 5,
	CAL_ADC_PUSHPULL_OUT = 6,
	CAL_ADC_BATTERY = 7
} calEntryNum_t;

#define CAL_NUMBER_OF_ENTRIES		8

/**
 * \brief Initializes the calibration data with default values
 */
void cal_Init(void);

void cal_Save(void);

void cal_Load(void);

int32_t cal_GetCalibratedValue(calEntryNum_t entry, int32_t rawValue);

void cal_UpdateEntry(calEntryNum_t entry, int32_t raw1, int32_t cal1,
		int32_t raw2, int32_t cal2);



#endif
