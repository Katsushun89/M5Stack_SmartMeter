#include "electric_bill_calculation.h"
#include "config_electric_bill.h"

ElectricBillCalculation::ElectricBillCalculation()
{
    power_consumption_this_month_ = 0;
    electric_bill_ = 0;  
    min_meter_read_power_ = 1000000;
}

int16_t ElectricBillCalculation::getDays(int16_t y, int16_t m, int16_t d)
{
    // 1・2月 → 前年の13・14月
    if(m <= 2){
        --y;
        m += 12;
    }
    int16_t dy = 365 * (y - 1); // 経過年数×365日
    int16_t c = y / 100;
    int16_t dl = (y >> 2) - c + (c >> 2); // うるう年分
    int16_t dm = (m * 979 - 1033) >> 5; // 1月1日から m 月1日までの日数
    return dy + dl + dm + d - 1;
}


int16_t ElectricBillCalculation::calcMeterReadingDiffDays(time_t *today)
{
    struct tm *tm_today;
    tm_today = localtime(today);
    int16_t y = tm_today->tm_year + 1900;
    int16_t m = tm_today->tm_mon + 1;
    int16_t d = tm_today->tm_mday;

    int16_t today_days = getDays(y, m, d);
    Serial.println("today:" + String(y) + "/" + String(m) + "/" + String(d));

    if(d <= METER_READING_DAY){
        m--;
    }
    int16_t meter_read_days = getDays(y, m, METER_READING_DAY);
    Serial.println("meter read day:" + String(y) + "/" + String(m) + "/" + String(METER_READING_DAY));

    int16_t diff_days = today_days - meter_read_days;
    Serial.println("today(c):" + String(today_days));
    Serial.println("meter_read_days(c):" + String(meter_read_days));
    Serial.println("diff days:" + String(diff_days));
    return diff_days;
}

void ElectricBillCalculation::setMeterReadingPowerConsumption(integral_power_record_t *last_meter_read_power)
{
    const uint32_t MIN_LIMIT = 12; 
    //何故か最初に1,2などがゴミデータで取得されるので除外する
    for(uint8_t i = 2; i < RECORD_SIZE; i++){
        if(last_meter_read_power->power_consumpution[i] > MIN_LIMIT &&
           last_meter_read_power->power_consumpution[i] < min_meter_read_power_){
               min_meter_read_power_ = last_meter_read_power->power_consumpution[i];
           }
    }
    Serial.println("min meter read " + String(min_meter_read_power_));
}

uint32_t ElectricBillCalculation::calcThisMonthPowerConsumption(integral_power_consumpution_t *latest_power)
{
    uint32_t latest_power_consumption = latest_power->power_consumpution;

    if(latest_power_consumption > min_meter_read_power_){
        power_consumption_this_month_ = latest_power_consumption - min_meter_read_power_;
        Serial.println("power consumption this month " + String(power_consumption_this_month_));
        return power_consumption_this_month_;
    }else{
        Serial.println("Error can not calc power consumption this month");
        Serial.println(String(latest_power_consumption) + "," + String(min_meter_read_power_));
        return 0;
    }
}

float ElectricBillCalculation::calcMeterRateLightingB(void)
{
    electric_bill_ = BASIC_CHARGE;

    if(power_consumption_this_month_ > PHASE_2_MAX_POWER_CONSUMPTION){
        electric_bill_ += (power_consumption_this_month_ - PHASE_2_MAX_POWER_CONSUMPTION) * PHASE_3_UNIT_CHARGE;
        electric_bill_ += (PHASE_2_MAX_POWER_CONSUMPTION - PHASE_1_MAX_POWER_CONSUMPTION) * PHASE_2_UNIT_CHARGE;
        electric_bill_ += (PHASE_1_MAX_POWER_CONSUMPTION) * PHASE_1_UNIT_CHARGE;
    }else if(power_consumption_this_month_ > PHASE_1_MAX_POWER_CONSUMPTION){
        electric_bill_ += (power_consumption_this_month_ - PHASE_1_MAX_POWER_CONSUMPTION) * PHASE_2_UNIT_CHARGE;
        electric_bill_ += (PHASE_1_MAX_POWER_CONSUMPTION) * PHASE_1_UNIT_CHARGE;
    }else if(power_consumption_this_month_ <= PHASE_1_MAX_POWER_CONSUMPTION){
        electric_bill_ += (power_consumption_this_month_) * PHASE_1_UNIT_CHARGE;
    }else{
        Serial.println("Error calc power consumption");
    }

    electric_bill_ += power_consumption_this_month_ * FUEL_COST_ADJUSTMENT;

    electric_bill_ += power_consumption_this_month_ * ADDITIONAL_LEVY_1_UNIT_CHARGE;

    electric_bill_ -= DISCOUNT_AMOUNT;
    
    Serial.println("calcMeterRateLightingB " + String(electric_bill_));
    return electric_bill_;
}

float ElectricBillCalculation::calcThisMonthElectricBill(void)
{
    switch(CONTRACTED_BILL_TYPE){
    case BILL_TYPE_METER_RATE_LIGHTING_B:
        return calcMeterRateLightingB();
    default:
        Serial.println("Error bill type");
        return 0;
    }
    return 0;
}

