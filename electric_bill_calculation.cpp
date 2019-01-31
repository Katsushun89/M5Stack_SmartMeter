#include "electric_bill_calculation.h"
#include "config_electric_bill.h"

ElectricBillCalculation::ElectricBillCalculation()
{

}

int32_t ElectricBillCalculation::getDays(int16_t y, int16_t m, int16_t d)
{
    // 1・2月 → 前年の13・14月
    if (m <= 2)
    {
      --y;
      m += 12;
    }
    int16_t dy = 365 * (y - 1); // 経過年数×365日
    int16_t c = y / 100;
    int16_t dl = (y >> 2) - c + (c >> 2); // うるう年分
    int16_t dm = (m * 979 - 1033) >> 5; // 1月1日から m 月1日までの日数
    return dy + dl + dm + d - 1;
}


int32_t ElectricBillCalculation::calcMeterReadingDiffDays(time_t *today)
{
    struct tm *tm_today;
    tm_today = localtime(today);
    int16_t y = tm_today->tm_year;
    int16_t m = tm_today->tm_mon;
    int16_t d = tm_today->tm_mday;

    int16_t today_days = getDays(y, m, d);

    if(d <= METER_READING_DAY){
        m--;
    }
    int16_t meter_read_days = getDays(y, m, METER_READING_DAY);

    int16_t diff_days = today_days - meter_read_days;
    Serial.printf("diff days %d", diff_days);
    return diff_days;
}

