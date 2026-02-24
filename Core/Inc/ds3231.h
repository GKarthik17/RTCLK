#ifndef __DS3231_H__
#define __DS3231_H__

#include "stm32f1xx_hal.h"   // or your MCU series
#include <stdint.h>

// I2C address for DS3231 (HAL wants 8-bit shifted)
#define DS3231_ADDRESS  (0x68 << 1)

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek;
    uint8_t day;
    uint8_t month;
    uint8_t year;      // last two digits
} DS3231_Time;

void DS3231_Init(I2C_HandleTypeDef *hi2c);

void DS3231_SetTime(DS3231_Time *t);
void DS3231_GetTime(DS3231_Time *t);

float DS3231_GetTemperature(void);

HAL_StatusTypeDef DS3231_SetAlarm1(I2C_HandleTypeDef *hi2c, uint8_t hour, uint8_t min, uint8_t sec);
void DS3231_ClearAlarm1(I2C_HandleTypeDef *hi2c);

#endif
