#include "Battery.h"
#include "common.h"

#include "file.h"

int8_t Battery_Load(Battery_t * bat, const char *filename) {
	if (file_open(filename, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
		return -1;
	}

	/* get number of battery data points */
	int16_t npoints = 0;
	const fileEntry_t num = { .name = "points", .ptr = &npoints, .type =
			PTR_INT16 };

	if (file_ReadParameters(&num, 1) != FILE_OK) {
		/* didn't find the number of points */
		file_close();
		return -2;
	}

	if (npoints < 1 || npoints > 200) {
		/* unreasonable number of points */
		file_close();
		return -3;
	}

	if(bat->profile) {
		/* Battery profile already allocated, free before loading new profile */
		vPortFree(bat->profile);
	}
	/* allocate memory for points */
	bat->profile = pvPortMalloc(npoints * sizeof(BatDataPoint_t));
	if (!bat->profile) {
		/* failed to allocate memory */
		file_close();
		return -4;
	}

	/* scan file for data points */
	uint16_t foundPoints = 0;

	do {
		/* find beginning of point */
		char line[30];
		do {
			if (!file_ReadLine(line, sizeof(line))) {
				/* file ended before enough points have been found */
				file_close();
				return -5;
			}
		} while (line[0] != '{');

		/* Read point entries */
		const fileEntry_t parms[5] = {
				{ .name = "SoC", .ptr =	&bat->profile[foundPoints].SoC, .type = PTR_INT32 },
				{ .name = "E", .ptr = &bat->profile[foundPoints].E, .type = PTR_INT32 },
				{ .name = "R1", .ptr = &bat->profile[foundPoints].R1, .type =	PTR_INT32 },
				{ .name = "R2", .ptr = &bat->profile[foundPoints].R2, .type = PTR_INT32 },
				{ .name = "C", .ptr = &bat->profile[foundPoints].C, .type = PTR_INT32 },
		};
		if (file_ReadParameters(parms, 5) != FILE_OK) {
			/* incomplete parameter list */
			vPortFree(bat->profile);
			file_close();
			return -6;
		}

		/* find end of point */
		do {
			if (!file_ReadLine(line, sizeof(line))) {
				/* file ended before enough points have been found */
				file_close();
				return -7;
			}
		} while (line[0] != '}');
		foundPoints++;
	} while (foundPoints < npoints);

	file_close();
	bat->npoints = foundPoints;
	/* got all points */
	return 0;
}

int8_t Battery_Save(Battery_t * bat, const char *filename) {
	if (file_open(filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		return -1;
	}

	/* write number of battery data points */
	const fileEntry_t num = { .name = "points", .ptr = &bat->npoints, .type =
			PTR_INT16 };

	file_WriteParameters(&num, 1);

	uint16_t writtenPoints;
	for(writtenPoints=0;writtenPoints<bat->npoints;writtenPoints++) {
		file_WriteLine("{\n");
		/* Write point entries */
		const fileEntry_t parms[5] = {
				{ .name = "SoC", .ptr =	&bat->profile[writtenPoints].SoC, .type = PTR_INT32 },
				{ .name = "E", .ptr = &bat->profile[writtenPoints].E, .type = PTR_INT32 },
				{ .name = "R1", .ptr = &bat->profile[writtenPoints].R1, .type =	PTR_INT32 },
				{ .name = "R2", .ptr = &bat->profile[writtenPoints].R2, .type = PTR_INT32 },
				{ .name = "C", .ptr = &bat->profile[writtenPoints].C, .type = PTR_INT32 },
		};
		file_WriteParameters(parms, 5);
		file_WriteLine("}\n");
	}

	file_close();
	return 0;
}

void Battery_Interpolate(const BatDataPoint_t *profile, uint16_t npoints,
		BatDataPoint_t *point) {
	/* find closest points */
	const BatDataPoint_t *lower = NULL;
	const BatDataPoint_t *upper = NULL;

	uint16_t i;
	for (i = 0; i < npoints; i++) {
		if (profile->SoC <= point->SoC) {
			if (!lower || (profile->SoC > lower->SoC)) {
				lower = profile;
			}
		}
		if (profile->SoC > point->SoC) {
			if (!upper || (profile->SoC < upper->SoC)) {
				upper = profile;
			}
		}
		profile++;
	}

	if (!lower && !upper) {
		/* this should not be possible */
		memset(point, 0, sizeof(BatDataPoint_t));
		return;
	}

	if (!lower) {
		/* only got an upper point */
		point->E = upper->E;
		point->R1 = upper->R1;
		point->R2 = upper->R2;
		point->C = upper->C;
	} else if (!upper) {
		/* only got a lower point */
		point->E = lower->E;
		point->R1 = lower->R1;
		point->R2 = lower->R2;
		point->C = lower->C;
	} else {
		/* interpolate */
		if (lower->SoC == upper->SoC) {
			/* this should not be possible */
			memset(point, 0, sizeof(BatDataPoint_t));
			return;
		}
		point->E = common_Map(point->SoC, lower->SoC, upper->SoC, lower->E,
				upper->E);
		point->R1 = common_Map(point->SoC, lower->SoC, upper->SoC, lower->R1,
				upper->R1);
		point->R2 = common_Map(point->SoC, lower->SoC, upper->SoC, lower->R2,
				upper->R2);
		point->C = common_Map(point->SoC, lower->SoC, upper->SoC, lower->C,
				upper->C);
	}
}

void Battery_Update(Battery_t *b, int32_t current) {
	/* update current battery capacity */
	static int64_t residualCharge = 0;
	int32_t timediff = xTaskGetTickCount() - b->lastUpdate;
	b->lastUpdate += timediff;
	residualCharge += current * timediff;
	if (abs(residualCharge) >= 3600000) {
		int32_t uAhdiff = residualCharge / 3600000;
		b->capacity -= uAhdiff;
		residualCharge -= (int64_t) uAhdiff * 3600000;
	}
	/* constrain capacity */
	if(b->capacity < 0) {
		b->capacity = 0;
	} else if(b->capacity > b->capacityFull) {
		b->capacity = b->capacityFull;
	}
	/* update SoC */
	b->state.SoC = (uint64_t) b->capacity * 100000000 / b->capacityFull;
	Battery_Interpolate(b->profile, b->npoints, &b->state);

	/* calculate current through battery capacitor C in uA */
	/* I_out = I_R2 + I_C
	 * I_R2 = U_R2 / R2
	 * U_R2 = -U_C
	 * => I_C = I_out + U_C / R2 */
	int32_t iC = current + ((int64_t) b->CVoltage * 1000000) / b->state.R2;

	/* update capacitor voltage */
	static int64_t residualCapCharge = 0;
	residualCapCharge += iC * timediff;
	if (abs(residualCapCharge) * 1000 > b->state.C) {
		int32_t uVdiff = (residualCapCharge * 1000) / b->state.C;
		b->CVoltage -= uVdiff;
		residualCapCharge -= (int64_t) uVdiff * b->state.C / 1000;
	}
}

void Battery_NewSoc(Battery_t *b, uint32_t SoC) {
	if (!b)
		/* no battery given */
		return;
	b->state.SoC = SoC;
	b->capacity = (uint64_t) b->capacityFull * SoC / 100000000;
	b->CVoltage = 0;
}

void Battery_NewCapacity(Battery_t *b, uint32_t capacity) {
	if (!b)
		/* no battery given */
		return;
	b->capacityFull = capacity;
	b->state.SoC = (uint64_t) b->capacity * 100000000 / b->capacityFull;
}
