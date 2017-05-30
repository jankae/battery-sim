#ifndef PUSHPULL_H_
#define PUSHPULL_H_

#include "cmsis_os.h"

#include "../Abstraction/calibration.h"

typedef struct {
	int32_t rawCurrentLow;
	int32_t rawCurrentHigh;
	uint32_t rawBatteryVoltage;
	uint32_t rawOutputVoltage;
	uint32_t rawBiasCurrent;
	uint32_t rawHighSideSupply;
} ppAvg_t;

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


#endif
