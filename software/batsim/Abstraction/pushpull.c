#include "../Abstraction/pushpull.h"

#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "gui.h"

#define PUSHPULL_DEFAULT_DRIVE		200

static PushPull_t output;

uint8_t pushpull_SPI_OK;

/* SPI block words */
#define SPI_COMMAND_WORD      	0
#define SPI_DAC1_CH_A         	1
#define SPI_DAC1_CH_B         	2
#define SPI_DAC2_CH_A         	3
#define SPI_DAC2_CH_B         	4
#define SPI_DAC3              	5
#define SPI_POT               	6

#define SPI_DAC_VOLTAGE			SPI_DAC1_CH_B
#define SPI_DAC_SOURCE			SPI_DAC2_CH_B
#define SPI_DAC_SINK			SPI_DAC2_CH_A

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
	output.control = NULL;
	output.currentChangeCB = NULL;
	pushpull_SPI_OK = 0;
	pushpull_AcquireControl();
	pushpull_SetDefault();
	pushpull_ReleaseControl();
}

void pushpull_AcquireControl(void) {
	if (output.control && output.control != xTaskGetCurrentTaskHandle()) {
		/* Currently controlled by another task -> send termination signal */
		xTaskNotify(output.control, SIGNAL_TERMINATE, eSetBits);
	}
	output.control = xTaskGetCurrentTaskHandle();
}

void pushpull_ReleaseControl(void) {
	/* Check if this task had control */
	if(output.control == xTaskGetCurrentTaskHandle()) {
		/* release control and set output stage to default values */
		pushpull_SetDefault();
		output.control = NULL;
	}
}

inline TaskHandle_t pushpull_GetControlHandle(void) {
	return output.control;
}

static uint16_t sampleRaw(const char * const title, uint16_t samples, uint16_t *source) {
	/* number of already sampled data */
	uint16_t n = 0;
	uint32_t sum = 0;

	/* Create a sampling window with nothing but a progressbar in it */
	window_t *w = window_new(title, Font_Big, COORDS(200, 40));
	progressbar_t *p = progressbar_new(window_GetAvailableArea(w));

	window_SetMainWidget(w, (widget_t*) p);

	pushpull_SetAveraging(0);

	do {
		uint32_t signal;
		if (App_Handler(&signal, portMAX_DELAY)) {
			if(signal & SIGNAL_PUSHPULL_UPDATE) {
				/* got a new sample */
				n++;
				sum += *source;
			}
			progressbar_SetState(p, (uint32_t) n * 100 / samples);
		}
	} while (n < samples);

	/* sampling competed */
	window_destroy(w);

	return sum / samples;
}

static int32_t getUserInputValue(const char * const title, const unit_t * const unit) {
	uint8_t ok = 0;
	int32_t val;
	do {
		char input[12];
		dialog_StringInput(title, input, sizeof(input));
		if(common_ValueFromString(&val, input, unit)) {
			/* got a valid user input */
			ok = 1;
		} else {
			dialog_MessageBox("ERROR", Font_Big, "Invalid input", MSG_OK, NULL,
					1);
		}
	} while(!ok);

	return val;
}

uint8_t pushpull_Calibrate(void) {
	pushpull_AcquireControl();

	if (dialog_MessageBox("Step 1", Font_Big, "Connect a voltmeter\nto the output",
			MSG_ABORT_OK, NULL, 1) != DIALOG_RESULT_OK) {
		/* abort calibration */
		pushpull_ReleaseControl();
		return 0;
	}

	/* Output is disabled, no current can flow */
	uint16_t zeroLow = sampleRaw("Offset I low", 2000, &output.rawCurrentLow);
	uint16_t zeroHigh = sampleRaw("Offset I high", 2000, &output.rawCurrentHigh);

	/* Set output to low value and enable */
	/* allow some current to flow */
	pushpull_SetSourceCurrent(5000);
	pushpull_SetSinkCurrent(5000);
	/* Set manually here, because it is important to know the exact DAC value */
	uint16_t dacVlow = CtrlWords[SPI_DAC_VOLTAGE] = DAC_MAX / 20;
	/* allow output to settle */
	vTaskDelay(100);
	uint16_t lowPushpull = sampleRaw("Low V internal", 2000, &output.rawOutputVoltage);
	/* Switch on the output to sample battery voltage */
	pushpull_SetEnabled(1);
	/* allow output to settle */
	vTaskDelay(100);
	uint16_t lowBattery = sampleRaw("Low V external", 2000, &output.rawBatteryVoltage);
	uint32_t lowActual = getUserInputValue("Voltage at output?", &Unit_Voltage);
	pushpull_SetEnabled(0);

	/* Set output to high value and enable */
	/* Set manually here, because it is important to know the exact DAC value */
	uint16_t dacVhigh = CtrlWords[SPI_DAC_VOLTAGE] = DAC_MAX - 1;
	/* allow output to settle */
	vTaskDelay(100);
	uint16_t highPushpull = sampleRaw("High V internal", 2000, &output.rawOutputVoltage);
	/* Switch on the output to sample battery voltage */
	pushpull_SetEnabled(1);
	/* allow output to settle */
	vTaskDelay(100);
	uint16_t highBattery = sampleRaw("High V external", 2000, &output.rawBatteryVoltage);
	uint32_t highActual = getUserInputValue("Voltage at output?", &Unit_Voltage);
	pushpull_SetEnabled(0);


	// TODO current calibration
	if (dialog_MessageBox("Step 2", Font_Big, "Short the output\nwith an ammeter",
			MSG_ABORT_OK, NULL, 1) != DIALOG_RESULT_OK) {
		/* abort calibration */
		pushpull_ReleaseControl();
		return 0;
	}

	/* Set a low voltage and limit the current allowed to flow */
	pushpull_SetVoltage(2000000);
	/* limit the current to approx. 250mA */
	uint16_t dacSource250 = CtrlWords[SPI_DAC_SOURCE] = DAC_MAX /12;
	vTaskDelay(10);
	pushpull_SetEnabled(1);
	vTaskDelay(100);
	while (pushpull_GetBatteryVoltage() > 1000000) {
		dialog_MessageBox("ERROR", Font_Big,
				"Output doesn't appear\nto be shorted.\n", MSG_OK, NULL, 1);
	}

	uint16_t lowCurrent250 = sampleRaw("Low current ADC", 2000, &output.rawCurrentLow);
	uint16_t highCurrent250 = sampleRaw("High current ADC", 2000, &output.rawCurrentHigh);
	uint32_t actualCurrent250 = getUserInputValue("Current across output?", &Unit_Current);
	pushpull_SetEnabled(0);

	if (dialog_MessageBox("Step 3", Font_Big, "Keep output shorted",
			MSG_ABORT_OK, NULL, 1) != DIALOG_RESULT_OK) {
		/* abort calibration */
		pushpull_ReleaseControl();
		return 0;
	}
	/* limit current to approx. 5mA */
	uint16_t dacSource10 = CtrlWords[SPI_DAC_SOURCE] = DAC_MAX / 300;
	vTaskDelay(10);
	pushpull_SetEnabled(1);
	vTaskDelay(100);
	uint16_t raw10mAlow = sampleRaw("Source I DAC", 2000, &output.rawCurrentLow);

	// TODO calibrate sink current DAC

	pushpull_ReleaseControl();
	/* Give GUI task some time to redraw active app */
	vTaskDelay(100);

	/* Calculate new calibration entries */
	cal_UpdateEntry(CAL_VOLTAGE_DAC, lowActual, dacVlow, highActual, dacVhigh);
	cal_UpdateEntry(CAL_ADC_PUSHPULL_OUT, lowPushpull, lowActual, highPushpull, highActual);
	cal_UpdateEntry(CAL_ADC_BATTERY, lowBattery, lowActual, highBattery, highActual);

	cal_UpdateEntry(CAL_ADC_CURRENT_LOW, zeroLow, 0, lowCurrent250, actualCurrent250);
	cal_UpdateEntry(CAL_ADC_CURRENT_HIGH, zeroHigh, 0, highCurrent250, actualCurrent250);

	/* calculate actual low current during source DAC calibration */
	int32_t actual10mA = cal_GetCalibratedValue(CAL_ADC_CURRENT_LOW, raw10mAlow);
	cal_UpdateEntry(CAL_MAX_CURRENT_DAC, actual10mA, dacSource10, actualCurrent250, dacSource250);

	/* TODO: Check calibration against default entries */
	return 1;
}

void pushpull_SetDefault(void) {
	pushpull_SetEnabled(0);
	pushpull_SetDriveCurrent(0);
	pushpull_SetVoltage(0);
	pushpull_SetSourceCurrent(0);
	pushpull_SetSinkCurrent(0);
}

void pushpull_SetAveraging(uint16_t samples) {
	output.averaging = samples;
	output.samplecount = 0;
	output.avgBatVoltage = 0;
	output.avgOutVoltage = 0;
	output.avgOutCurrent = 0;
}


void pushpull_SetVoltage(uint32_t uv) {
	if (xTaskGetCurrentTaskHandle() != output.control)
		return;
	int32_t val = 0;
	if (uv > MAX_VOLTAGE) {
		uv = MAX_VOLTAGE;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_VOLTAGE_DAC, uv);

	output.voltage = uv;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC_VOLTAGE] = val;
}

void pushpull_SetSourceCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != output.control)
		return;
	int32_t val = 0;
	if (ua >= MAX_SOURCE_CURRENT) {
		ua = MAX_SOURCE_CURRENT;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_MAX_CURRENT_DAC, ua);

	output.currentLimitSource = ua;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC_SOURCE] = val;
}

void pushpull_SetSinkCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != output.control)
		return;
	int32_t val = 0;
	if (ua >= MAX_SINK_CURRENT) {
		ua = MAX_SINK_CURRENT;
	}
	/* calculate DAC value for given voltage */
	val = cal_GetCalibratedValue(CAL_MIN_CURRENT_DAC, ua);

	output.currentLimitSink = ua;
	if (val < 0)
		val = 0;
	if (val >= 65535)
		val = 65535;
	CtrlWords[SPI_DAC_SINK] = val;
}

void pushpull_SetEnabled(uint8_t enabled) {
	if (xTaskGetCurrentTaskHandle() != output.control)
		return;
	if (enabled) {
		output.enabled = 1;
		CtrlWords[SPI_COMMAND_WORD] |= SPI_COMMAND_OUTPUT;
	} else {
		output.enabled = 0;
		CtrlWords[SPI_COMMAND_WORD] &= ~SPI_COMMAND_OUTPUT;
	}
}

inline uint8_t pushpull_GetEnabled(void){
	return output.enabled;
}

void pushpull_SetDriveCurrent(uint32_t ua) {
	if (xTaskGetCurrentTaskHandle() != output.control)
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
	if (xTaskGetCurrentTaskHandle() != output.control)
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
	return output.outputCurrent;
}
inline uint32_t pushpull_GetOutputVoltage(void) {
	return output.outputVoltage;
}
inline uint32_t pushpull_GetBatteryVoltage(void) {
	return output.batteryVoltage;
}
inline uint32_t pushpull_GetBiasCurrent(void) {
	return output.biasCurrent;
}
inline int8_t pushpull_GetTemperature(void) {
	return output.temperature;
}
inline uint16_t pushpull_GetRawCurrentLow(void) {
	return output.rawCurrentLow;
}
inline uint16_t pushpull_GetRawCurrentHigh(void) {
	return output.rawCurrentHigh;
}
inline uint16_t pushpull_GetRawBatteryVoltage(void) {
	return output.rawBatteryVoltage;
}
inline uint16_t pushpull_GetRawOutputVoltage(void) {
	return output.rawOutputVoltage;
}
inline uint16_t pushpull_GetRawBiasCurrent(void) {
	return output.rawBiasCurrent;
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

		output.rawCurrentLow = RawADC[ADC_LOW_CURRENT];
		output.rawCurrentHigh = RawADC[ADC_HIGH_CURRENT];
		output.rawBatteryVoltage = RawADC[ADC_BATTERY];
		output.rawOutputVoltage = RawADC[ADC_PUSHPULL_OUT];
		output.rawBiasCurrent = RawADC[ADC_BIAS_CURRENT];
		/* Convert raw ADC values */
		int32_t battery = cal_GetCalibratedValue(CAL_ADC_BATTERY, RawADC[ADC_BATTERY]);
		int32_t voltage = cal_GetCalibratedValue(CAL_ADC_PUSHPULL_OUT,
				RawADC[ADC_PUSHPULL_OUT]);
		if (battery < 0) {
			battery = 0;
		}
		if (voltage < 0) {
			voltage = 0;
		}
		int32_t current;
		if((RawADC[ADC_LOW_CURRENT] > 200) && (RawADC[ADC_LOW_CURRENT] < ADC_MAX_SINGLE - 200)) {
			/* Low current sense is not saturated */
			current = cal_GetCalibratedValue(CAL_ADC_CURRENT_LOW, RawADC[ADC_LOW_CURRENT]);
		} else {
			/* use high current sense as low sense is saturated */
			current = cal_GetCalibratedValue(CAL_ADC_CURRENT_HIGH, RawADC[ADC_HIGH_CURRENT]);
		}
		if(output.averaging) {
			output.avgBatVoltage += battery;
			output.avgOutVoltage += voltage;
			output.avgOutCurrent += current;
			output.samplecount++;
			if(output.samplecount >= output.averaging) {
				/* one averaging cycle has finished */
				output.batteryVoltage = output.avgBatVoltage / output.samplecount;
				output.outputVoltage = output.avgOutVoltage / output.samplecount;
				output.outputCurrent = output.avgOutCurrent / output.samplecount;
				output.samplecount = 0;
				output.avgBatVoltage = 0;
				output.avgOutVoltage = 0;
				output.avgOutCurrent = 0;
				if(output.control) {
					BaseType_t yield;
					xTaskNotifyFromISR(output.control, SIGNAL_PUSHPULL_UPDATE,
							eSetValueWithOverwrite, &yield);
					portYIELD_FROM_ISR(yield);
				}
			}
		} else {
			/* no averaging -> use values directly */
			output.batteryVoltage = battery;
			output.outputVoltage = voltage;
			output.outputCurrent = current;
			if(output.control) {
				BaseType_t yield;
				xTaskNotifyFromISR(output.control, SIGNAL_PUSHPULL_UPDATE,
						eSetBits, &yield);
				portYIELD_FROM_ISR(yield);
			}
		}
		/* No calibration for bias current available */
		// TODO add zero compensation for voltage dependency
		/* Bias current is measured as the voltage drop over 0.2 Ohms amplified by 10
		 * -> ADC full scale (3.3V) corresponds to 1.65A */
		// TODO at full scale this overflows but bias current must never be that high
		output.biasCurrent = (RawADC[ADC_BIAS_CURRENT] * 1650000UL) / ADC_MAX_SINGLE;

		/* convert to °C, full scale ADC is about 393K */
		output.temperature = (uint32_t) RawADC[0] * 393 / 4096 - 273; // and subtract °C to K difference
	}
}

