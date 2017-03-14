#include "../Abstraction/pushpull.h"

#define PUSHPULL_CURRENT_ADC_RATE			1400000
#define PUSHPULL_CURRENT_UPDATE_RATE		1000
#define PUSHPULL_CURRENT_SAMPLES			((2*PUSHPULL_CURRENT_ADC_RATE/PUSHPULL_CURRENT_UPDATE_RATE)&0xfffffffe)

#define PUSHPULL_OTHERS_ADC_RATE			134615
#define PUSHPULL_OTHERS_UPDATE_RATE			1000
#define PUSHPULL_OTHERS_SAMPLES				((PUSHPULL_OTHERS_ADC_RATE/PUSHPULL_OTHERS_UPDATE_RATE)&0xfffffffc)

#define PUSHPULL_DEFAULT_DRIVE		200
#define PUSHPULL_DEFAULT_DROPOUT	3000000

#define VOLTAGE_RANGE_LOW()					(GPIOE->BSRR = GPIO_PIN_7<<16u)
#define VOLTAGE_RANGE_HIGH()				(GPIOE->BSRR = GPIO_PIN_7)
#define OUTPUT_ENABLE()						(GPIOE->BSRR = GPIO_PIN_6<<16u)
#define OUTPUT_DISABLE()					(GPIOE->BSRR = GPIO_PIN_6)
#define BUCK_ENABLE()						(GPIOE->BSRR = GPIO_PIN_15<<16u)
#define BUCK_DISABLE()						(GPIOE->BSRR = GPIO_PIN_15)

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern DAC_HandleTypeDef hdac;

struct {
	/* Controlling task */
	TaskHandle_t control;
	/* 'should' voltage of the Push-Pull-Stage */
	uint32_t voltage;
	/* maximum current allowed to flow */
	uint32_t currentLimit;
	/* Current flowing in(-) and out(+) of the Push-Pull-Stage */
	int32_t outputCurrent;
	/* Regulated voltage at the PP-Output */
	uint32_t outputVoltage;
	/* Voltage at the battery terminals */
	uint32_t batteryVoltage;
	/* Bias current through the high- and low-side (only accurate while output is off) */
	uint32_t biasCurrent;
	/* Supply voltage at the PP-Stage */
	uint32_t highSideSupply;
	int32_t transferredCharge;
	/* raw ADC values */
	int32_t rawCurrentLow;
	int32_t rawCurrentHigh;
	uint32_t rawBatteryVoltage;
	uint32_t rawOutputVoltage;
	uint32_t rawBiasCurrent;
	uint32_t rawHighSideSupply;

	/* high/low voltage mode */
	pushpullMode_t mode;
	/* output state */
	uint8_t enabled;

	/* callback function called whenever a new current value is available */
	void (*currentChangeCB)(int32_t newCurrent);

	/* state variables for measuring average ADC values */
	struct {
		/* average measurement state */
		volatile uint8_t enabled;
		/* requested current samples */
		uint16_t minSamplesCurrent;
		/* requested 'other' samples */
		uint16_t minSamplesOthers;
		/* sampled current values */
		uint16_t samplesCurrent;
		/* sampled 'other' values */
		uint16_t samplesOthers;
		/* sum of sampled values */
		ppAvg_t values;
	} avg;
} pushpull;

/* ADC current values (filled by DMA) */
uint32_t rawADCCurrent[PUSHPULL_CURRENT_SAMPLES];
/* ADC 'other' values (filled by DMA) */
uint16_t rawADCOthers[PUSHPULL_OTHERS_SAMPLES];

void pushpull_Init(void) {
	BUCK_ENABLE();
	extDAC_Init();
	pushpull_SetEnabled(0);
	pushpull_SetDriveCurrent(PUSHPULL_DEFAULT_DRIVE);
	pushpull_SetDropoutVoltage(PUSHPULL_DEFAULT_DROPOUT);
	pushpull.control = NULL;
	pushpull.outputCurrent = 0;
	pushpull.transferredCharge = 0;
	pushpull.currentChangeCB = NULL;
	pushpull.enabled = 0;
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*) rawADCOthers,
			PUSHPULL_OTHERS_SAMPLES);
	HAL_ADC_Start(&hadc2);
	HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) rawADCCurrent,
			PUSHPULL_CURRENT_SAMPLES);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	pushpull_SetVoltage(0);
	pushpull_SetCurrentLimit(0);
}

void pushpull_AcquireControl(void) {
	if (pushpull.control) {
		/* Currently controlled by another task -> send termination signal */
		xTaskNotify(pushpull.control, 15, eSetValueWithOverwrite);
	}
	pushpull.control = xTaskGetCurrentTaskHandle();
}

void pushpull_ReleaseControl(void) {
	pushpull.control = NULL;
}

void pushpull_RecalibrateZeroCurrent(void) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	/* save current settings */
	uint8_t enabled = pushpull.enabled;
	pushpullMode_t mode = pushpull.mode;
	uint32_t voltage = pushpull.voltage;
	uint32_t currentLimit = pushpull.currentLimit;
	void (*cb)(int32_t) = pushpull.currentChangeCB;
	pushpull.currentChangeCB = NULL;
	/* Switch output off */
	pushpull_SetEnabled(0);
	pushpull_SetMode(PP_HIGH_VOLTAGE);

	/* raw measurements at calibration points */
	ppAvg_t point1, point2;

	/* set first calibration point */
	pushpull_SetVoltage(1000000);
	pushpull_SetCurrentLimit(100000);
	HAL_Delay(5);
	point1 = pushpull_GetAverageRaw(100, 100);

	/* set second calibration point */
	pushpull_SetVoltage(16000000);
	HAL_Delay(5);
	point2 = pushpull_GetAverageRaw(100, 100);

	/* update calibration values */
	cal_UpdateEntry(CAL_ADC_CURRENT_OFFSET_LOW, point1.rawOutputVoltage,
			point1.rawCurrentLow, point2.rawOutputVoltage,
			point2.rawCurrentLow);
	cal_UpdateEntry(CAL_ADC_CURRENT_OFFSET_HIGH, point1.rawOutputVoltage,
			point1.rawCurrentHigh, point2.rawOutputVoltage,
			point2.rawCurrentHigh);

	/* set old values again */
	pushpull_SetMode(mode);
	pushpull_SetVoltage(voltage);
	pushpull_SetCurrentLimit(currentLimit);
	pushpull.currentChangeCB = cb;
	pushpull_SetEnabled(enabled);
}

ppAvg_t pushpull_GetAverageRaw(uint16_t minCurrentSamples,
		uint16_t minOthersSamples) {
	/* reset last result */
	memset(&pushpull.avg, 0, sizeof(pushpull.avg));
	/* set requested number of samples */
	pushpull.avg.minSamplesCurrent = minCurrentSamples;
	pushpull.avg.minSamplesOthers = minOthersSamples;
	/* start sampling */
	pushpull.avg.enabled = 1;
	/* wait for sampling (in interrupt) to finish */
	while (pushpull.avg.enabled)
		;
	/* calculate average values */
	pushpull.avg.values.rawCurrentHigh /= pushpull.avg.samplesCurrent;
	pushpull.avg.values.rawCurrentLow /= pushpull.avg.samplesCurrent;
	pushpull.avg.values.rawBatteryVoltage /= pushpull.avg.samplesOthers;
	pushpull.avg.values.rawBiasCurrent /= pushpull.avg.samplesOthers;
	pushpull.avg.values.rawHighSideSupply /= pushpull.avg.samplesOthers;
	pushpull.avg.values.rawOutputVoltage /= pushpull.avg.samplesOthers;
	/* return result */
	return pushpull.avg.values;
}

void pushpull_SetMode(pushpullMode_t mode) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	pushpull.mode = mode;
	if(mode==PP_HIGH_VOLTAGE) {
		VOLTAGE_RANGE_HIGH();
	} else {
		VOLTAGE_RANGE_LOW();
	}
	/* Update voltage */
	pushpull_SetVoltage(pushpull.voltage);
}

void pushpull_SetVoltage(uint32_t uv) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	int32_t val = 0;
	switch (pushpull.mode) {
	case PP_HIGH_VOLTAGE:
		if (uv > MAX_VOLTAGE_HIGH_SCALE) {
			uv = MAX_VOLTAGE_HIGH_SCALE;
		}
		/* calculate DAC value for given voltage */
		val = cal_GetCalibratedValue(CAL_VOLTAGE_DAC_HIGH, uv);
		break;
	case PP_LOW_VOLTAGE:
		if (uv > MAX_VOLTAGE_LOW_SCALE) {
			uv = MAX_VOLTAGE_LOW_SCALE;
		}
		/* calculate DAC value for given voltage */
		val = cal_GetCalibratedValue(CAL_VOLTAGE_DAC_LOW, uv);
		break;
	}
	pushpull.voltage = uv;
	if (val < 0)
		val = 0;
	if (val >= 4096)
		val = 4095;
	/* load channel 2 with calculated value */
	DAC->DHR12R2 = val;
}

void pushpull_SetCurrentLimit(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	/* convert to DAC value */
	/* max. Limit is 3030303uA */
	if (ua >= 3030303) {
		ua = 3030303;
	}
	pushpull.currentLimit = ua;
	ua /= 740;
	DAC->DHR12R1 = ua;
}

void pushpull_SetEnabled(uint8_t enabled){
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	if(enabled) {
		pushpull.enabled = 1;
		OUTPUT_ENABLE();
	} else {
		pushpull.enabled = 0;
		OUTPUT_DISABLE();
	}
}

inline uint8_t pushpull_GetEnabled(void){
	return pushpull.enabled;
}

void pushpull_SetDriveCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	/* limit drive current to 1.9mA */
	if (ua >= 1900)
		ua = 1900;
	/* calculate necessary DAC value */
	/* 0.6V transistor drop + outputCurrent through 1k resistor */
	uint32_t Uout = 600 + ua;
	uint32_t DACvalue = (Uout * 4095) / 2048;
	extDAC_Set(EXTDAC_CHANNEL_B, DACvalue);
}

void pushpull_SetDropoutVoltage(uint32_t uv) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	/* dropout voltage must be within 0.6V to 5.6V */
	if (uv < 600000)
		uv = 600000;
	else if (uv > 5600000)
		uv = 5600000;
	/* calculate necessary DAC value */
	/* uv = 0.6V -> DAC = 2.5V; uv = 5.6V -> DAC = 0V */
	uint32_t Uout = (5000 - (uv / 1000 - 600)) / 2;
	uint32_t DACvalue = (Uout * 4095) / 2048;
	extDAC_Set(EXTDAC_CHANNEL_A, DACvalue);
}

inline int32_t pushpull_GetCurrent(void) {
	return pushpull.outputCurrent;
}

inline uint32_t pushpull_GetOutputVoltage(void) {
	return pushpull.outputVoltage;
}

inline uint32_t pushpull_GetBatteryVoltage(void) {
	return pushpull.batteryVoltage;
}

inline uint32_t pushpull_GetBiasCurrent(void) {
	return pushpull.biasCurrent;
}

inline uint32_t pushpull_GetSupplyVoltage(void) {
	return pushpull.highSideSupply;
}

static void pushpull_UpdateCurrent(uint32_t *adcStart) {
	/* PUSHPULL_SAMPLES / 2 new current samples */
	/* two ADC channels (low current/high current) simultaneously sampled */
	/* calculate average value for both channels and choose the appropriate
	 * channel per sample
	 */
	int32_t highSum = 0, lowSum = 0, highUsedSum = 0, lowUsedSum = 0;
	uint32_t nUsedLow = 0, nUsedHigh = 0;
	uint32_t i;
	for (i = 0; i < PUSHPULL_CURRENT_SAMPLES / 2; i++) {
		int32_t high = -(int32_t) (adcStart[i] & 0xffff) + 2048;
		int32_t low = -(int32_t) (adcStart[i] >> 16) + 2048;
		highSum += high;
		lowSum += low;
		if (low > -2000 || low < 2000) {
			/* low channel is not at limit -> use this as it has higher resolution */
			lowUsedSum += low;
			nUsedLow++;
		} else {
			/* low channel saturated -> use high channel */
			highUsedSum += high;
			nUsedHigh++;
		}
	}
	/* save average channel values */
	pushpull.rawCurrentHigh = highSum
			/ (int32_t) (PUSHPULL_CURRENT_SAMPLES / 2);
	pushpull.rawCurrentLow = lowSum / (int32_t) (PUSHPULL_CURRENT_SAMPLES / 2);
	/* convert to outputCurrent */
	/* get average of used values */
	if (nUsedHigh) {
		highUsedSum /= nUsedHigh;
	}
	if (nUsedLow) {
		lowUsedSum /= nUsedLow;
	}
	/* subtract voltage dependent offset */
	highUsedSum -= cal_GetCalibratedValue(CAL_ADC_CURRENT_OFFSET_HIGH,
			pushpull.rawOutputVoltage);
	lowUsedSum -= cal_GetCalibratedValue(CAL_ADC_CURRENT_OFFSET_LOW,
			pushpull.rawOutputVoltage);
	/* convert to outputCurrent */
	int32_t currentLow = cal_GetCalibratedValue(CAL_ADC_CURRENT_FACTOR_LOW,
			lowUsedSum);
	int32_t currentHigh = cal_GetCalibratedValue(CAL_ADC_CURRENT_FACTOR_HIGH,
			highUsedSum);
	/* average low and high current results */
	pushpull.outputCurrent = (((int64_t) currentLow * nUsedLow)
			+ ((int64_t) currentHigh * nUsedHigh))
			/ (PUSHPULL_CURRENT_SAMPLES / 2);
	// TODO update transferred charge
	if (pushpull.currentChangeCB) {
		pushpull.currentChangeCB(pushpull.outputCurrent);
	}
	/* update average values if sampling is running */
	if (pushpull.avg.enabled) {
		pushpull.avg.values.rawCurrentHigh += pushpull.rawCurrentHigh;
		pushpull.avg.values.rawCurrentLow += pushpull.rawCurrentLow;
		pushpull.avg.samplesCurrent++;
		if (pushpull.avg.samplesCurrent >= pushpull.avg.minSamplesCurrent
				&& pushpull.avg.samplesOthers
						>= pushpull.avg.minSamplesOthers) {
			pushpull.avg.enabled = 0;
		}
	}
}

static void pushpull_UpdateOthers(void) {
	/* get average of sampled channels */
	uint16_t sum[4] = { 0, 0, 0, 0 };
	uint8_t i;
	for (i = 0; i < PUSHPULL_OTHERS_SAMPLES; i += 4) {
		sum[0] += rawADCOthers[i];
		sum[1] += rawADCOthers[i + 1];
		sum[2] += rawADCOthers[i + 2];
		sum[3] += rawADCOthers[i + 3];
	}
	sum[0] /= PUSHPULL_OTHERS_SAMPLES / 4;
	sum[1] /= PUSHPULL_OTHERS_SAMPLES / 4;
	sum[2] /= PUSHPULL_OTHERS_SAMPLES / 4;
	sum[3] /= PUSHPULL_OTHERS_SAMPLES / 4;
	/* store raw average values */
	pushpull.rawOutputVoltage = sum[0];
	pushpull.rawBiasCurrent = sum[1];
	pushpull.rawHighSideSupply = sum[2];
	pushpull.rawBatteryVoltage = sum[3];

	/* convert to voltages/currents */
	pushpull.batteryVoltage = cal_GetCalibratedValue(CAL_ADC_BATTERY,
			pushpull.rawBatteryVoltage);
	pushpull.outputVoltage = cal_GetCalibratedValue(CAL_ADC_PUSHPULL_OUT,
			pushpull.rawOutputVoltage);

	/* high side supply is not calibrated -> convert directly */
	/* voltage divider 1/11, ADC ref 2.5, 4096 values ->
	 * 2.5V/4096*11 => 6713.867188uV/LSB */
	pushpull.highSideSupply = pushpull.rawHighSideSupply * 6713.867188f;

	/* bias current is not calibrated -> convert directly */
	/* Sense shunt 0.1Ohm, amplified by 100 => 10V/A
	 * 2.5/4096/10 => 61.03515625uA/LSB */
	pushpull.biasCurrent = pushpull.rawBiasCurrent * 61.03515625f;

	/* update average values if sampling is running */
	if (pushpull.avg.enabled) {
		pushpull.avg.values.rawBatteryVoltage += pushpull.rawBatteryVoltage;
		pushpull.avg.values.rawBiasCurrent += pushpull.rawBiasCurrent;
		pushpull.avg.values.rawHighSideSupply += pushpull.rawHighSideSupply;
		pushpull.avg.values.rawOutputVoltage += pushpull.rawOutputVoltage;
		pushpull.avg.samplesOthers++;
		if (pushpull.avg.samplesCurrent >= pushpull.avg.minSamplesCurrent
				&& pushpull.avg.samplesOthers
						>= pushpull.avg.minSamplesOthers) {
			pushpull.avg.enabled = 0;
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc->Instance == ADC1 || hadc->Instance == ADC2) {
		/* the second half of the samples has been updated */
		pushpull_UpdateCurrent(&rawADCCurrent[PUSHPULL_CURRENT_SAMPLES / 2]);
	} else if (hadc->Instance == ADC3) {
		pushpull_UpdateOthers();
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc->Instance == ADC1 || hadc->Instance == ADC2) {
		/* the first half of the samples has been updated */
		pushpull_UpdateCurrent(&rawADCCurrent[0]);
	}
}
