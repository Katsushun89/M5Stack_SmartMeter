#pragma once
#include <M5Stack.h>
#include "electric_define.h"

class ElectricBillCalculation{
private:
    int16_t getDays(int16_t y, int16_t m, int16_t d);
    uint32_t power_consumption_this_month_;
    float electric_bill_;
    uint32_t min_meter_read_power_;
    float calcMeterRateLightingB(void);

public:
    ElectricBillCalculation();
    int16_t calcMeterReadingDiffDays(time_t *today);
    void setMeterReadingPowerConsumption(integral_power_record_t *last_meter_read_power);
    uint32_t calcThisMonthPowerConsumption(integral_power_consumpution_t *latest_power);
    float calcThisMonthElectricBill(void);
};
