#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include <stdint.h>
#include <string.h>

/* Defines for automatic calculation of default calibration values */
#define MAX_VOLTAGE					19314705
#define MAX_SOURCE_CURRENT			3000000
#define MAX_SINK_CURRENT			3000000
#define MAX_CURRENT_LOW_ADC			300000
#define MAX_CURRENT_HIGH_ADC		3000000
#define MAX_PUSHPULL_OUT			28050000
#define MAX_BATTERY					28576596

#define DAC_MAX						65536
#define ADC_MAX_SINGLE				4096
#define ADC_MAX_DIFF				2048

typedef enum {
	CAL_VOLTAGE_DAC = 0,
	CAL_MAX_CURRENT_DAC = 1,
	CAL_MIN_CURRENT_DAC = 2,
	CAL_ADC_CURRENT_LOW = 3,
	CAL_ADC_CURRENT_HIGH = 4,
	CAL_ADC_PUSHPULL_OUT = 5,
	CAL_ADC_BATTERY = 6
} calEntryNum_t;

#define CAL_NUMBER_OF_ENTRIES		7

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
