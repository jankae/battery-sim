#ifndef PUSHPULL_H_
#define PUSHPULL_H_

#include "cmsis_os.h"

#include "../Abstraction/calibration.h"

#define SPI_BLOCK_SIZE				10

typedef struct {
	int32_t rawCurrentLow;
	int32_t rawCurrentHigh;
	uint32_t rawBatteryVoltage;
	uint32_t rawOutputVoltage;
	uint32_t rawBiasCurrent;
	uint32_t rawHighSideSupply;
} ppAvg_t;

typedef struct {
	/* Controlling task */
	TaskHandle_t control;
	/* 'should' voltage of the Push-Pull-Stage */
	uint32_t voltage;
	/* maximum current allowed to flow */
	uint32_t currentLimitSource;
	uint32_t currentLimitSink;
	/* Current flowing in(-) and out(+) of the Push-Pull-Stage */
	int32_t outputCurrent;
	/* Regulated voltage at the PP-Output */
	uint32_t outputVoltage;
	/* Voltage at the battery terminals */
	uint32_t batteryVoltage;
	/* Bias current through the high- and low-side (only accurate while output is off) */
	uint32_t biasCurrent;
	int32_t transferredCharge;
	/* raw ADC values */
	int32_t rawCurrentLow;
	int32_t rawCurrentHigh;
	uint32_t rawBatteryVoltage;
	uint32_t rawOutputVoltage;
	uint32_t rawBiasCurrent;

	/* output state */
	uint8_t enabled;

	/* callback function called whenever a new current value is available */
	void (*currentChangeCB)(int32_t newCurrent);

	/* state variables for measuring average ADC values */
	struct {
		/* average measurement state */
		volatile uint8_t enabled;
		/* number of requested samples */
		uint16_t samples;
		/* sum of sampled values */
		ppAvg_t values;
	} avg;
} PushPull_t;

typedef struct {
	int32_t minVoltage;
	int32_t maxVoltage;
	int32_t maxCurrent;
	int32_t minCurrent;
} PushPull_Limits_t;

extern const PushPull_Limits_t Limits;

/**
 * \brief Initializes the Push-Pull-Stage, Output switched off
 *
 * - State variables set to initial values
 * - ADC + DMA sampling process started
 */
void pushpull_Init(void);

void pushpull_AcquireControl(void);

void pushpull_ReleaseControl(void);

///**
// * \brief Re-calibrates the zero-current-point
// *
// * The output is switched off briefly while a simple 2-point calibration
// * for the zero-current-point (depends on the output voltage) is executed.
// * At the end, the original settings are restored.
// */
//void pushpull_RecalibrateZeroCurrent(void);

ppAvg_t pushpull_GetAverageRaw(uint16_t samples);

void pushpull_SetVoltage(uint32_t uv);

void pushpull_SetSourceCurrent(uint32_t ua);

void pushpull_SetSinkCurrent(uint32_t ua);

void pushpull_SetEnabled(uint8_t enabled);

uint8_t pushpull_GetEnabled(void);

void pushpull_SetDriveCurrent(uint32_t ua);

int32_t pushpull_GetCurrent(void);

uint32_t pushpull_GetOutputVoltage(void);

uint32_t pushpull_GetBatteryVoltage(void);

uint32_t pushpull_GetBiasCurrent(void);

uint32_t pushpull_GetSupplyVoltage(void);

void pushpull_SPITransfer(void);
void pushpull_SPIComplete(void);

#endif
