#include "common.h"

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
