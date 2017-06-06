#include "../Abstraction/calibration.h"

typedef struct {
	int32_t offset;
	float scale;
} calEntryData_t;

const calEntryData_t defaultEntries[CAL_NUMBER_OF_ENTRIES] = {
		/* voltage to DAC */
		{0, (float) DAC_MAX / MAX_VOLTAGE},
		/* source current */
		{0, (float) DAC_MAX / MAX_SOURCE_CURRENT},
		/* sink current */
		{0, (float) DAC_MAX / MAX_SINK_CURRENT},
		/* ADC to current low */
		{ADC_MAX_DIFF, (float) -MAX_CURRENT_LOW_ADC / ADC_MAX_DIFF},
		/* ADC to current high */
		{ADC_MAX_DIFF, (float) -MAX_CURRENT_HIGH_ADC / ADC_MAX_DIFF},
		/* ADC to push/pull out */
		{0, (float) MAX_PUSHPULL_OUT / ADC_MAX_SINGLE},
		/* ADC to battery voltage */
		{0, (float) MAX_BATTERY / ADC_MAX_SINGLE},
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
	rawValue -= entries[entry].offset;
	return entries[entry].scale * rawValue;
}

void cal_UpdateEntry(calEntryNum_t entry, int32_t raw1, int32_t cal1,
		int32_t raw2, int32_t cal2){
	/* calculate scale */
	entries[entry].scale = (float) (cal2 - cal1) / (raw2 - raw1);
	/* calculate offset */
	entries[entry].offset = cal1 - raw1 * entries[entry].scale;
}
