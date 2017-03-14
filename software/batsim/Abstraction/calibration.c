#include "../Abstraction/calibration.h"

typedef struct {
	float offset;
	float scale;
} calEntryData_t;

const calEntryData_t defaultEntries[CAL_NUMBER_OF_ENTRIES] = {
		/* low voltage to DAC */
		{0.0f, (float) DAC_MAX / MAX_VOLTAGE_LOW_SCALE},
		/* high voltage to DAC */
		{0.0f, (float) DAC_MAX / MAX_VOLTAGE_HIGH_SCALE},
		/* ADC to current offset low */
		{0.0f, 0.0f},
		/* ADC to current offset high */
		{0.0f, 0.0f},
		/* ADC to current factor low */
		{0.0f, (float) MAX_CURRENT_LOW_ADC / ADC_MAX_DIFF},
		/* ADC to current factor high */
		{0.0f, (float) MAX_CURRENT_HIGH_ADC / ADC_MAX_DIFF},
		/* ADC to push/pull out */
		{0.0f, (float) MAX_PUSHPULL_OUT / ADC_MAX_SINGLE},
		/* ADC to battery voltage */
		{0.0f, (float) MAX_BATTERY / ADC_MAX_SINGLE},
};

calEntryData_t entries[CAL_NUMBER_OF_ENTRIES];

void cal_Init(void){
	/* start with default entries */
	memcpy(entries, defaultEntries, sizeof(entries));
}

void cal_Save(void){
	// TODO save calibration data to SD card
}

void cal_Load(void){
	// TODO load calibration data from SD card
}

int32_t cal_GetCalibratedValue(calEntryNum_t entry, int32_t rawValue) {
	return entries[entry].scale * rawValue + entries[entry].offset;
}

void cal_UpdateEntry(calEntryNum_t entry, int32_t raw1, int32_t cal1,
		int32_t raw2, int32_t cal2){
	/* calculate scale */
	entries[entry].scale = (float) (cal2 - cal1) / (raw2 - raw1);
	/* calculate offset */
	entries[entry].offset = cal1 - raw1 * entries[entry].scale;
}
