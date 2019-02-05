#pragma once

#include <stdint.h>

const uint8_t METER_READING_DAY = 12;


//https://www.kakeibo.tepco.co.jp/ratesim/

typedef enum {
    BILL_TYPE_METER_RATE_LIGHTING_B = 0,
} BILL_TYPE_T;

const BILL_TYPE_T CONTRACTED_BILL_TYPE = BILL_TYPE_METER_RATE_LIGHTING_B;

// 従量電灯Ｂ meter rate lighting
const float BASIC_CHARGE = 1123.00;

const float PHASE_1_UNIT_CHARGE = 19.52; //最初の120kWhまで（第１段階料金）
const float PHASE_2_UNIT_CHARGE = 26.00; //120kWhをこえ300kWhまで（第２段階料金）
const float PHASE_3_UNIT_CHARGE = 30.02; //上記超過（第３段階料金）
const float FUEL_COST_ADJUSTMENT = -0.66; //燃料費調整額

const float PHASE_1_MAX_POWER_CONSUMPTION = 120; //[kWh]
const float PHASE_2_MAX_POWER_CONSUMPTION = 300; //[kWh]

const float ADDITIONAL_LEVY_1_UNIT_CHARGE = 2.90; //再生可能エネルギー発電促進賦課金

const float DISCOUNT_AMOUNT = 54; //口座振替割引額	
