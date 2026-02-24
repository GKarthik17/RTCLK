#include "ds3231.h"

static I2C_HandleTypeDef *ds3231_i2c;

// --------------------------------------------------
// Helper: BCD <-> DEC
// --------------------------------------------------
static uint8_t BCD2DEC(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

static uint8_t DEC2BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// --------------------------------------------------
// Initialize driver with selected I2C
// --------------------------------------------------
void DS3231_Init(I2C_HandleTypeDef *hi2c) {
    ds3231_i2c = hi2c;
}

// --------------------------------------------------
// Set date & time
// --------------------------------------------------
void DS3231_SetTime(DS3231_Time *t) {
    uint8_t data[7];

    data[0] = DEC2BCD(t->seconds);
    data[1] = DEC2BCD(t->minutes);
    data[2] = DEC2BCD(t->hours);
    data[3] = DEC2BCD(t->dayOfWeek);
    data[4] = DEC2BCD(t->day);
    data[5] = DEC2BCD(t->month);
    data[6] = DEC2BCD(t->year);

    HAL_I2C_Mem_Write(ds3231_i2c, DS3231_ADDRESS,
                      0x00, I2C_MEMADD_SIZE_8BIT,
                      data, 7, 1000);
}

// --------------------------------------------------
// Get date & time
// --------------------------------------------------
void DS3231_GetTime(DS3231_Time *t) {
    uint8_t data[7];

    HAL_I2C_Mem_Read(ds3231_i2c, DS3231_ADDRESS,
                     0x00, I2C_MEMADD_SIZE_8BIT,
                     data, 7, 1000);

    t->seconds   = BCD2DEC(data[0] & 0x7F);
    t->minutes   = BCD2DEC(data[1] & 0x7F);
    t->hours     = BCD2DEC(data[2] & 0x3F);
    t->dayOfWeek = BCD2DEC(data[3]);
    t->day       = BCD2DEC(data[4]);
    t->month     = BCD2DEC(data[5] & 0x1F);
    t->year      = BCD2DEC(data[6]);
}

// --------------------------------------------------
// Read internal temperature
// --------------------------------------------------
float DS3231_GetTemperature(void) {
    uint8_t tempData[2];

    HAL_I2C_Mem_Read(ds3231_i2c, DS3231_ADDRESS,
                     0x11, I2C_MEMADD_SIZE_8BIT,
                     tempData, 2, 1000);

    int8_t msb  = tempData[0];
    uint8_t lsb = tempData[1];

    return msb + (lsb >> 6) * 0.25f;
}

HAL_StatusTypeDef DS3231_SetAlarm1(I2C_HandleTypeDef *hi2c,
                                   uint8_t hour,
                                   uint8_t min,
                                   uint8_t sec)
{
    uint8_t data[4];

    /* Match seconds, minutes, hours — ignore day/date */
    data[0] = DecToBCD(sec);        // A1M1 = 0
    data[1] = DecToBCD(min);        // A1M2 = 0
    data[2] = DecToBCD(hour);       // A1M3 = 0
    data[3] = 0x80;                 // A1M4 = 1 → ignore day/date

    /* Write alarm registers */
    if (HAL_I2C_Mem_Write(hi2c, 0x68 << 1, 0x07,
                          I2C_MEMADD_SIZE_8BIT, data, 4, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    /* Enable Alarm1 interrupt in control register */
    uint8_t ctrl;
    HAL_I2C_Mem_Read(hi2c, 0x68 << 1, 0x0E,
                     I2C_MEMADD_SIZE_8BIT, &ctrl, 1, HAL_MAX_DELAY);

    ctrl |= 0x05;   // INTCN=1, A1IE=1

    return HAL_I2C_Mem_Write(hi2c, 0x68 << 1, 0x0E,
                             I2C_MEMADD_SIZE_8BIT, &ctrl, 1, HAL_MAX_DELAY);
}

void DS3231_ClearAlarm1(I2C_HandleTypeDef *hi2c)
{
    uint8_t status;

    HAL_I2C_Mem_Read(hi2c, 0x68 << 1, 0x0F,
                     I2C_MEMADD_SIZE_8BIT, &status, 1, HAL_MAX_DELAY);

    status &= ~0x01;   // Clear A1F flag

    HAL_I2C_Mem_Write(hi2c, 0x68 << 1, 0x0F,
                      I2C_MEMADD_SIZE_8BIT, &status, 1, HAL_MAX_DELAY);
}
