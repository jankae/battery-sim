#ifndef PUSHPULL_H_
#define PUSHPULL_H_

#include "cmsis_os.h"

#include "../Abstraction/calibration.h"

#define SPI_BLOCK_SIZE				10

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
	/* Heatsink temperature in °C */
	int8_t temperature;
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

	uint16_t averaging;
	uint16_t samplecount;

	uint64_t avgBatVoltage;
	uint64_t avgOutVoltage;
	int64_t avgOutCurrent;

	/* callback function called whenever a new current value is available */
	void (*currentChangeCB)(int32_t newCurrent);
} PushPull_t;

typedef struct {
	int32_t minVoltage;
	int32_t maxVoltage;
	int32_t maxCurrent;
	int32_t minCurrent;
	int32_t maxResistance;
	int32_t minResistance;
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
TaskHandle_t pushpull_GetControlHandle(void);

void pushpull_SetDefault(void);

void pushpull_SetAveraging(uint16_t samples);

void pushpull_SetVoltage(uint32_t uv);
void pushpull_SetSourceCurrent(uint32_t ua);
void pushpull_SetSinkCurrent(uint32_t ua);
void pushpull_SetEnabled(uint8_t enabled);
void pushpull_SetDriveCurrent(uint32_t ua);
void pushpull_SetInternalResistance(uint32_t ur);

uint8_t pushpull_GetEnabled(void);
int32_t pushpull_GetCurrent(void);
uint32_t pushpull_GetOutputVoltage(void);
uint32_t pushpull_GetBatteryVoltage(void);
uint32_t pushpull_GetBiasCurrent(void);
uint32_t pushpull_GetSupplyVoltage(void);
int8_t pushpull_GetTemperature(void);

void pushpull_SPITransfer(void);
void pushpull_SPIComplete(void);

#endif
