#pragma once
#include <M5Stack.h>

class ElectricBillCalculation{
private:
    int32_t getDays(int16_t y, int16_t m, int16_t d);


public:
    ElectricBillCalculation();
    int32_t calcMeterReadingDiffDays(time_t *today);
};
