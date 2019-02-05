#pragma once

typedef uint8_t accumulation_collection_date_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t sec;
    uint32_t power_consumpution;
} integral_power_consumpution_t;

#define RECORD_SIZE (48)

typedef struct {
    uint16_t record_date_diff;
    uint32_t power_consumpution[RECORD_SIZE];
} integral_power_record_t;

