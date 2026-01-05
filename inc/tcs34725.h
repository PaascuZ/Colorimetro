#ifndef TCS34725_H
#define TCS34725_H

#include <stdint.h>
#include <stdbool.h>

/* =====================
 * I2C address
 * ===================== */
#define TCS34725_I2C_ADDR   0x29

/* =====================
 * Register addresses
 * ===================== */
#define TCS34725_CMD_BIT    0x80

#define TCS34725_REG_ENABLE 0x00
#define TCS34725_REG_ATIME  0x01
#define TCS34725_REG_CONTROL 0x0F

#define TCS34725_REG_CDATAL 0x14
#define TCS34725_REG_RDATAL 0x16
#define TCS34725_REG_GDATAL 0x18
#define TCS34725_REG_BDATAL 0x1A

/* =====================
 * ENABLE register bits
 * ===================== */
#define TCS34725_ENABLE_PON 0x01
#define TCS34725_ENABLE_AEN 0x02

/* =====================
 * Integration times
 * ===================== */
typedef enum {
    TCS34725_IT_2_4MS  = 0xFF,
    TCS34725_IT_24MS   = 0xF6,
    TCS34725_IT_50MS   = 0xEB,
    TCS34725_IT_154MS  = 0xC0,
    TCS34725_IT_700MS  = 0x00
} tcs34725_it_t;

/* =====================
 * Gain values
 * ===================== */
typedef enum {
    TCS34725_GAIN_1X  = 0x00,
    TCS34725_GAIN_4X  = 0x01,
    TCS34725_GAIN_16X = 0x02,
    TCS34725_GAIN_60X = 0x03
} tcs34725_gain_t;

/* =====================
 * Raw data struct
 * ===================== */
typedef struct {
    uint16_t c;
    uint16_t r;
    uint16_t g;
    uint16_t b;
} tcs34725_raw_t;

/* =====================
 * API
 * ===================== */
bool tcs34725_init(void);

void tcs34725_enable(bool en);

void tcs34725_set_integration_time(tcs34725_it_t it);
void tcs34725_set_gain(tcs34725_gain_t gain);

bool tcs34725_read_raw(tcs34725_raw_t *out);

#endif // TCS34725_H
