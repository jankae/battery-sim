#include "common.h"

static const unitElement_t uA = {"uA", 1};
static const unitElement_t mA = {"mA", 1000};
static const unitElement_t A = {"A", 1000000};

const unit_t Unit_Current = {&uA, &mA, &A, NULL};

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
