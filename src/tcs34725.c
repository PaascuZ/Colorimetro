#include "tcs34725.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"
#include "utils.h"
#include "tcs34725.h"

/* =====================
 * I2C helpers
 * ===================== */
static bool tcs_write8(uint8_t reg, uint8_t value);
static bool tcs_read16(uint8_t reg, uint16_t *value);

/* =====================
 * API
 * ===================== */
bool tcs34725_init(void)
{
    // Test presenza sensore: prova a scrivere ENABLE = 0
    if (!tcs_write8(TCS34725_REG_ENABLE, 0x00)) {
        return false;
    }

    // Power ON
    tcs34725_enable(true);

    // Piccolo delay post power-on (datasheet)
    utils_delay_ms(3);

    return true;
}

void tcs34725_enable(bool en)
{
    if (en) {
        tcs_write8(TCS34725_REG_ENABLE,
                   TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    } else {
        tcs_write8(TCS34725_REG_ENABLE, 0x00);
    }
}

void tcs34725_set_integration_time(tcs34725_it_t it)
{
    tcs_write8(TCS34725_REG_ATIME, (uint8_t)it);
}

void tcs34725_set_gain(tcs34725_gain_t gain)
{
    tcs_write8(TCS34725_REG_CONTROL, (uint8_t)gain);
}

bool tcs34725_read_raw(tcs34725_raw_t *out)
{
    if (!out) return false;

    if (!tcs_read16(TCS34725_REG_CDATAL, &out->c)) return false;
    if (!tcs_read16(TCS34725_REG_RDATAL, &out->r)) return false;
    if (!tcs_read16(TCS34725_REG_GDATAL, &out->g)) return false;
    if (!tcs_read16(TCS34725_REG_BDATAL, &out->b)) return false;

    return true;
}

/* =====================
 * I2C low-level
 * ===================== */
static bool tcs_write8(uint8_t reg, uint8_t value)
{
    if (!i2c_start()) return false;

    if (!i2c_write((TCS34725_I2C_ADDR << 1) | 0)) {
        i2c_stop();
        return false;
    }

    if (!i2c_write(TCS34725_CMD_BIT | reg)) {
        i2c_stop();
        return false;
    }

    if (!i2c_write(value)) {
        i2c_stop();
        return false;
    }

    i2c_stop();
    return true;
}

static bool tcs_read16(uint8_t reg, uint16_t *value)
{
    uint8_t low, high;

    if (!i2c_start()) return false;

    if (!i2c_write((TCS34725_I2C_ADDR << 1) | 0)) {
        i2c_stop();
        return false;
    }

    if (!i2c_write(TCS34725_CMD_BIT | reg)) {
        i2c_stop();
        return false;
    }

    if (!i2c_restart()) return false;

    if (!i2c_write((TCS34725_I2C_ADDR << 1) | 1)) {
        i2c_stop();
        return false;
    }

    low  = i2c_read(true);
    high = i2c_read(false);

    i2c_stop();

    *value = ((uint16_t)high << 8) | low;
    return true;
}
