#include "tcs34725.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"
#include "utils.h"

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

    // Delay post power-on (datasheet)
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

/* ============================================================
 * RAW (0..65535) -> RGB 8-bit (0..255)
 *
 * Nota importante:
 * - Il TCS34725 NON nasce ?0..255?, i registri sono 16-bit.
 * - Per avere un RGB ?classico?, normalizziamo rispetto al clear C:
 *     r8 = (R * 255) / C
 *     g8 = (G * 255) / C
 *     b8 = (B * 255) / C
 * - Clamp a 255 e gestione C=0.
 * ============================================================ */
void tcs34725_raw_to_rgb8(const tcs34725_raw_t *in, uint8_t *r8, uint8_t *g8, uint8_t *b8)
{
    if (!in || !r8 || !g8 || !b8) return;

    if (in->c == 0u) {
        *r8 = 0; *g8 = 0; *b8 = 0;
        return;
    }

    // (opzionale) evita valori > C (può succedere per rumore/aritmetica)
    uint32_t c = (uint32_t)in->c;
    uint32_t r = (in->r > in->c) ? c : (uint32_t)in->r;
    uint32_t g = (in->g > in->c) ? c : (uint32_t)in->g;
    uint32_t b = (in->b > in->c) ? c : (uint32_t)in->b;

    // rounding: +c/2
    uint32_t rr = (r * 255u + (c / 2u)) / c;
    uint32_t gg = (g * 255u + (c / 2u)) / c;
    uint32_t bb = (b * 255u + (c / 2u)) / c;

    if (rr > 255u) rr = 255u;
    if (gg > 255u) gg = 255u;
    if (bb > 255u) bb = 255u;

    *r8 = (uint8_t)rr;
    *g8 = (uint8_t)gg;
    *b8 = (uint8_t)bb;
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
