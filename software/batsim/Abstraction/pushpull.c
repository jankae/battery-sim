#include "../Abstraction/pushpull.h"

#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

#define PUSHPULL_DEFAULT_DRIVE		200

static PushPull_t pushpull;

uint8_t pushpull_SPI_OK;

/* SPI block words */
#define SPI_COMMAND_WORD      	0
#define SPI_DAC1_CH_A         	1
#define SPI_DAC1_CH_B         	2
#define SPI_DAC2_CH_A         	3
#define SPI_DAC2_CH_B         	4
#define SPI_DAC3              	5
#define SPI_POT               	6

/* SPI command word flags */
#define SPI_COMMAND_OUTPUT    	0x01

/* Raw ADC word meanings */
#define ADC_TEMPERATURE			0
#define ADC_BATTERY				1
#define ADC_PUSHPULL_OUT		2
#define ADC_LOW_CURRENT			3
#define ADC_HIGH_CURRENT		4
#define ADC_BIAS_CURRENT		5

const PushPull_Limits_t Limits = {
		.minVoltage = 0,
		.maxVoltage = MAX_VOLTAGE,
		.minCurrent = MAX_SINK_CURRENT,
		.maxCurrent = MAX_SOURCE_CURRENT,
		.maxResistance = MAX_RESISTANCE,
		.minResistance = MIN_RESISTANCE,
};

uint16_t CtrlWords[SPI_BLOCK_SIZE];
uint16_t RawADC[SPI_BLOCK_SIZE];

extern SPI_HandleTypeDef hspi1;

void pushpull_Init(void) {
	pushpull.control = NULL;
	pushpull.outputCurrent = 0;
	pushpull.transferredCharge = 0;
	pushpull.currentChangeCB = NULL;
	pushpull.enabled = 0;
	pushpull_SPI_OK = 0;
	pushpull_AcquireControl();
	pushpull_SetDefault();
	pushpull_ReleaseControl();
}

void pushpull_AcquireControl(void) {
	if (pushpull.control) {
		/* Currently controlled by another task -> send termination signal */
		xTaskNotify(pushpull.control, SIGNAL_TERMINATE, eSetBits);
	}
	pushpull.control = xTaskGetCurrentTaskHandle();
}

void pushpull_ReleaseControl(void) {
	/* Check if this task had control */
	if(pushpull.control == xTaskGetCurrentTaskHandle()) {
		/* release control and set pushpull stage to default values */
		pushpull_SetDefault();
		pushpull.control = NULL;
	}
}

inline TaskHandle_t pushpull_GetControlHandle(void) {
	return pushpull.control;
}

void pushpull_SetDefault(void) {
	pushpull_SetEnabled(0);
	pushpull_SetDriveCurrent(0);
	pushpull_SetVoltage(0);
	pushpull_SetSourceCurrent(0);
	pushpull_SetSinkCurrent(0);
}

void pushpull_SetAveraging(uint16_t samples) {
	pushpull.averaging = samples;
	pushpull.samplecount = 0;
	pushpull.avgBatVoltage = 0;
	pushpull.avgOutVoltage = 0;
	pushpull.avgOutCurrent = 0;
}


void pushpull_SetVoltage(uint32_t uv) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	int32_t val = 0;
	if (uv > MAX_VOLTAGE) {
		uv = MAX_VOLTAGE;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_VOLTAGE_DAC, uv);

	pushpull.voltage = uv;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC1_CH_B] = val;
}

void pushpull_SetSourceCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	int32_t val = 0;
	if (ua >= MAX_SOURCE_CURRENT) {
		ua = MAX_SOURCE_CURRENT;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_MAX_CURRENT_DAC, ua);

	pushpull.currentLimitSource = ua;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC2_CH_B] = val;
}

void pushpull_SetSinkCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	int32_t val = 0;
	if (ua >= MAX_SINK_CURRENT) {
		ua = MAX_SINK_CURRENT;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_MIN_CURRENT_DAC, ua);

	pushpull.currentLimitSink = ua;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC2_CH_A] = val;
}

void pushpull_SetEnabled(uint8_t enabled) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;
	if (enabled) {
		pushpull.enabled = 1;
		CtrlWords[SPI_COMMAND_WORD] |= SPI_COMMAND_OUTPUT;
	} else {
		pushpull.enabled = 0;
		CtrlWords[SPI_COMMAND_WORD] &= ~SPI_COMMAND_OUTPUT;
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
	/* TODO: 0.6V transistor drop + outputCurrent through 1k resistor */
	uint32_t Uout = 500 + ua;
	uint32_t DACvalue = (Uout * DAC_MAX) / 3300;
	CtrlWords[SPI_DAC1_CH_A] = DACvalue;
}

void pushpull_SetInternalResistance(uint32_t ur) {
	if (xTaskGetCurrentTaskHandle() != pushpull.control)
		return;

/* Shunt has nominal value of 0.11 Ohm */
#define R_SHUNT 110000UL
/* Use a simplified model for the digital potentiometer setting
 * the internal resistance. Values (especially for large R_i) won't
 * be accurate */
//#define POT_MODEL_SIMPLE
#ifdef POT_MODEL_SIMPLE

	/* R = R_shunt * (255-Pot_val)/Pot_val
	 * =>
	 * Pot_val = R_shunt*255/(R+R_shunt)
	 */
	uint8_t potval = R_SHUNT * 255 / (ur + R_SHUNT);
	CtrlWords[SPI_POT] = potval;
#else
/* Use more complex potentiometer model, see datasheet (MCP41HVX1) */
#define R_ZS			80
#define R_FS			60
#define R_S				19.06f

	/* R = R_shunt * (R_FS+R_S*(255-potval))/(R_ZS+R_S*potval)
	 * =>
	 * potval = (R_shunt*R_FS/R_S+R_shunt*255+R*R_ZS/R_S)/(R+R_shunt)
	 */
#define MAX_RI	(int)(R_SHUNT*(R_FS+R_S*255)/R_ZS)
#define MIN_RI	(int)(R_SHUNT*R_FS/(R_ZS+R_S*255)+1)
	if (ur < MIN_RI) {
		ur = MIN_RI;
	} else if (ur > MAX_RI) {
		ur = MAX_RI;
	}
	uint8_t potval = (R_SHUNT * R_FS / R_S + 255 * R_SHUNT + R_ZS / R_S * ur)
			/ (ur + R_SHUNT);
	CtrlWords[SPI_POT] = potval;
#endif
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

inline int8_t pushpull_GetTemperature(void) {
	return pushpull.temperature;
}

inline void pushpull_SPITransfer(void) {
	HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t*) CtrlWords,
			(uint8_t*) RawADC, SPI_BLOCK_SIZE);
}

void pushpull_SPIComplete(void) {
	uint8_t i;
	for (i = 0; i < SPI_BLOCK_SIZE; i++) {
		if (RawADC[i] >= 4096) {
			/* this must be a transmission error as raw ADC value can't be that high */
			pushpull_SPI_OK = 0;
			break;
		}
	}
	if (i == SPI_BLOCK_SIZE) {
		pushpull_SPI_OK = 1;

		/* Convert raw ADC values */
		uint32_t battery = cal_GetCalibratedValue(CAL_ADC_BATTERY, RawADC[ADC_BATTERY]);
		uint32_t voltage = cal_GetCalibratedValue(CAL_ADC_PUSHPULL_OUT, RawADC[ADC_PUSHPULL_OUT]);
		int32_t current;
		if((RawADC[ADC_LOW_CURRENT] > 200) && (RawADC[ADC_LOW_CURRENT] < ADC_MAX_SINGLE - 200)) {
			/* Low current sense is not saturated */
			current = cal_GetCalibratedValue(CAL_ADC_CURRENT_LOW, RawADC[ADC_LOW_CURRENT]);
		} else {
			/* use high current sense as low sense is saturated */
			current = cal_GetCalibratedValue(CAL_ADC_CURRENT_HIGH, RawADC[ADC_HIGH_CURRENT]);
		}
		if(pushpull.averaging) {
			pushpull.avgBatVoltage += battery;
			pushpull.avgOutVoltage += voltage;
			pushpull.avgOutCurrent += current;
			pushpull.samplecount++;
			if(pushpull.samplecount >= pushpull.averaging) {
				/* one averaging cycle has finished */
				pushpull.batteryVoltage = pushpull.avgBatVoltage / pushpull.samplecount;
				pushpull.outputVoltage = pushpull.avgOutVoltage / pushpull.samplecount;
				pushpull.outputCurrent = pushpull.avgOutCurrent / pushpull.samplecount;
				pushpull.samplecount = 0;
				pushpull.avgBatVoltage = 0;
				pushpull.avgOutVoltage = 0;
				pushpull.avgOutCurrent = 0;
				if(pushpull.control) {
					BaseType_t yield;
					xTaskNotifyFromISR(pushpull.control, SIGNAL_PUSHPULL_UPDATE,
							eSetValueWithOverwrite, &yield);
					portYIELD_FROM_ISR(yield);
				}
			}
		} else {
			/* no averaging -> use values directly */
			pushpull.batteryVoltage = battery;
			pushpull.outputVoltage = voltage;
			pushpull.outputCurrent = current;
			if(pushpull.control) {
				BaseType_t yield;
				xTaskNotifyFromISR(pushpull.control, SIGNAL_PUSHPULL_UPDATE,
						eSetBits, &yield);
				portYIELD_FROM_ISR(yield);
			}
		}
		/* No calibration for bias current available */
		// TODO add zero compensation for voltage dependency
		/* Bias current is measured as the voltage drop over 0.2 Ohms amplified by 10
		 * -> ADC full scale (3.3V) corresponds to 1.65A */
		// TODO at full scale this overflows but bias current must never be that high
		pushpull.biasCurrent = (RawADC[ADC_BIAS_CURRENT] * 1650000UL) / ADC_MAX_SINGLE;

		/* convert to °C, full scale ADC is about 393K */
		pushpull.temperature = (uint32_t) RawADC[0] * 393 / 4096 - 273; // and subtract °C to K difference
	}
}

//static void pushpull_UpdateCurrent(uint32_t *adcStart) {
//	/* PUSHPULL_SAMPLES / 2 new current samples */
//	/* two ADC channels (low current/high current) simultaneously sampled */
//	/* calculate average value for both channels and choose the appropriate
//	 * channel per sample
//	 */
//	int32_t highSum = 0, lowSum = 0, highUsedSum = 0, lowUsedSum = 0;
//	uint32_t nUsedLow = 0, nUsedHigh = 0;
//	uint32_t i;
//	for (i = 0; i < PUSHPULL_CURRENT_SAMPLES / 2; i++) {
//		int32_t high = -(int32_t) (adcStart[i] & 0xffff) + 2048;
//		int32_t low = -(int32_t) (adcStart[i] >> 16) + 2048;
//		highSum += high;
//		lowSum += low;
//		if (low > -2000 || low < 2000) {
//			/* low channel is not at limit -> use this as it has higher resolution */
//			lowUsedSum += low;
//			nUsedLow++;
//		} else {
//			/* low channel saturated -> use high channel */
//			highUsedSum += high;
//			nUsedHigh++;
//		}
//	}
//	/* save average channel values */
//	pushpull.rawCurrentHigh = highSum
//			/ (int32_t) (PUSHPULL_CURRENT_SAMPLES / 2);
//	pushpull.rawCurrentLow = lowSum / (int32_t) (PUSHPULL_CURRENT_SAMPLES / 2);
//	/* convert to outputCurrent */
//	/* get average of used values */
//	if (nUsedHigh) {
//		highUsedSum /= nUsedHigh;
//	}
//	if (nUsedLow) {
//		lowUsedSum /= nUsedLow;
//	}
//	/* subtract voltage dependent offset */
//	highUsedSum -= cal_GetCalibratedValue(CAL_ADC_CURRENT_OFFSET_HIGH,
//			pushpull.rawOutputVoltage);
//	lowUsedSum -= cal_GetCalibratedValue(CAL_ADC_CURRENT_OFFSET_LOW,
//			pushpull.rawOutputVoltage);
//	/* convert to outputCurrent */
//	int32_t currentLow = cal_GetCalibratedValue(CAL_ADC_CURRENT_FACTOR_LOW,
//			lowUsedSum);
//	int32_t currentHigh = cal_GetCalibratedValue(CAL_ADC_CURRENT_FACTOR_HIGH,
//			highUsedSum);
//	/* average low and high current results */
//	pushpull.outputCurrent = (((int64_t) currentLow * nUsedLow)
//			+ ((int64_t) currentHigh * nUsedHigh))
//			/ (PUSHPULL_CURRENT_SAMPLES / 2);
//	// TODO update transferred charge
//	if (pushpull.currentChangeCB) {
//		pushpull.currentChangeCB(pushpull.outputCurrent);
//	}
//	/* update average values if sampling is running */
//	if (pushpull.avg.enabled) {
//		pushpull.avg.values.rawCurrentHigh += pushpull.rawCurrentHigh;
//		pushpull.avg.values.rawCurrentLow += pushpull.rawCurrentLow;
//		pushpull.avg.samplesCurrent++;
//		if (pushpull.avg.samplesCurrent >= pushpull.avg.minSamplesCurrent
//				&& pushpull.avg.samplesOthers
//						>= pushpull.avg.minSamplesOthers) {
//			pushpull.avg.enabled = 0;
//		}
//	}
//}
//
//static void pushpull_UpdateOthers(void) {
//	/* get average of sampled channels */
//	uint16_t sum[4] = { 0, 0, 0, 0 };
//	uint8_t i;
//	for (i = 0; i < PUSHPULL_OTHERS_SAMPLES; i += 4) {
//		sum[0] += rawADCOthers[i];
//		sum[1] += rawADCOthers[i + 1];
//		sum[2] += rawADCOthers[i + 2];
//		sum[3] += rawADCOthers[i + 3];
//	}
//	sum[0] /= PUSHPULL_OTHERS_SAMPLES / 4;
//	sum[1] /= PUSHPULL_OTHERS_SAMPLES / 4;
//	sum[2] /= PUSHPULL_OTHERS_SAMPLES / 4;
//	sum[3] /= PUSHPULL_OTHERS_SAMPLES / 4;
//	/* store raw average values */
//	pushpull.rawOutputVoltage = sum[0];
//	pushpull.rawBiasCurrent = sum[1];
//	pushpull.rawHighSideSupply = sum[2];
//	pushpull.rawBatteryVoltage = sum[3];
//
//	/* convert to voltages/currents */
//	pushpull.batteryVoltage = cal_GetCalibratedValue(CAL_ADC_BATTERY,
//			pushpull.rawBatteryVoltage);
//	pushpull.outputVoltage = cal_GetCalibratedValue(CAL_ADC_PUSHPULL_OUT,
//			pushpull.rawOutputVoltage);
//
//	/* high side supply is not calibrated -> convert directly */
//	/* voltage divider 1/11, ADC ref 2.5, 4096 values ->
//	 * 2.5V/4096*11 => 6713.867188uV/LSB */
//	pushpull.highSideSupply = pushpull.rawHighSideSupply * 6713.867188f;
//
//	/* bias current is not calibrated -> convert directly */
//	/* Sense shunt 0.1Ohm, amplified by 100 => 10V/A
//	 * 2.5/4096/10 => 61.03515625uA/LSB */
//	pushpull.biasCurrent = pushpull.rawBiasCurrent * 61.03515625f;
//
//	/* update average values if sampling is running */
//	if (pushpull.avg.enabled) {
//		pushpull.avg.values.rawBatteryVoltage += pushpull.rawBatteryVoltage;
//		pushpull.avg.values.rawBiasCurrent += pushpull.rawBiasCurrent;
//		pushpull.avg.values.rawHighSideSupply += pushpull.rawHighSideSupply;
//		pushpull.avg.values.rawOutputVoltage += pushpull.rawOutputVoltage;
//		pushpull.avg.samplesOthers++;
//		if (pushpull.avg.samplesCurrent >= pushpull.avg.minSamplesCurrent
//				&& pushpull.avg.samplesOthers
//						>= pushpull.avg.minSamplesOthers) {
//			pushpull.avg.enabled = 0;
//		}
//	}
//}
//
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//	if (hadc->Instance == ADC1 || hadc->Instance == ADC2) {
//		/* the second half of the samples has been updated */
//		pushpull_UpdateCurrent(&rawADCCurrent[PUSHPULL_CURRENT_SAMPLES / 2]);
//	} else if (hadc->Instance == ADC3) {
//		pushpull_UpdateOthers();
//	}
//}
//
//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
//	if (hadc->Instance == ADC1 || hadc->Instance == ADC2) {
//		/* the first half of the samples has been updated */
//		pushpull_UpdateCurrent(&rawADCCurrent[0]);
//	}
//}
