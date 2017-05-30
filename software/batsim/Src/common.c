#include "common.h"

static const unitElement_t uA = { "uA", 1 };
static const unitElement_t mA = { "mA", 1000 };
static const unitElement_t A = { "A", 1000000 };

static const unitElement_t uV = { "uV", 1 };
static const unitElement_t mV = { "mV", 1000 };
static const unitElement_t V = { "V", 1000000 };

const unit_t Unit_Current = { &uA, &mA, &A, NULL };

const unit_t Unit_Voltage = { &uV, &mV, &V, NULL };

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
		int32_t scaleToLow, int32_t scaleToHigh) {
	int32_t result;
	value -= scaleFromLow;
	int32_t rangeFrom = scaleFromHigh - scaleFromLow;
	int32_t rangeTo = scaleToHigh - scaleToLow;
	result = ((int64_t) value * rangeTo) / rangeFrom;
	result += scaleToLow;
	return result;
}

void common_StringFromValue(char *to, uint8_t len, int32_t val,
		const unit_t * const unit) {
	/* store sign */
	int8_t negative = 0;
	if (val < 0) {
		val = -val;
		negative = 1;
	}
	/* find applicable unit */
	const unitElement_t *selectedUnit = (*unit)[0];
	uint8_t i = 1;
	while ((*unit)[i]) {
		if (val / (*unit)[i]->factor) {
			/* this unit is a better fit */
			selectedUnit = (*unit)[i];
		}
		i++;
	}
	if (!selectedUnit) {
		/* this should not be possible */
		*to = 0;
		return;
	}
	/* calculate number of available digits */
	uint8_t digits = len - strlen(selectedUnit->name) - negative;
	/* calculate digits before the dot */
	uint32_t intval = val / selectedUnit->factor;
	uint8_t beforeDot = 0;
	while (intval) {
		intval /= 10;
		beforeDot++;
	}
	if (beforeDot > digits) {
		/* value does not fit available space */
		*to = 0;
		return;
	}
	if(!beforeDot)
		beforeDot = 1;
	uint8_t spaceAfter = digits - 1 - beforeDot;
	uint32_t factor = 1;
	int8_t afterDot = 0;
	while (spaceAfter && factor < selectedUnit->factor) {
		factor *= 10;
		spaceAfter--;
		afterDot++;
	}
	val = ((uint64_t) val * factor) / selectedUnit->factor;

	/* compose string from the end */
	/* copy unit */
	uint8_t pos = digits + negative;
	strcpy(&to[pos], selectedUnit->name);
	/* actually displayed digits */
	digits = beforeDot + afterDot;
	while(digits) {
		afterDot--;
		digits--;
		to[--pos] = val % 10 + '0';
		val /= 10;
		if(afterDot==0) {
			/* place dot at this position */
			to[--pos] = '.';
		}
	}
	if(negative) {
		to[--pos] = '-';
	}
	while(pos>0) {
		to[--pos] = ' ';
	}
}
