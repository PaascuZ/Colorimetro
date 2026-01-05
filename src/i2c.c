#include "i2c.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

//#include "config_bits.h"
#include "i2c.h"
#include "clock.h"

// =====================
// Config I2C
// =====================
#define I2C_BAUDRATE   100000UL   // 100 kHz

// I2CxBRG = (PBCLK / (2 * Fsck)) - 2
#define I2C_BRG_VALUE  ((PBCLK_HZ / (2UL * I2C_BAUDRATE)) - 2UL)

// =====================
// Helper locali
// =====================
static bool i2c_wait_idle(void);

// =====================
// API
// =====================
void i2c_init(void)
{
    // Disabilita I2C durante setup
    I2C1CONbits.ON = 0;

    // Baud rate
    I2C1BRG = I2C_BRG_VALUE;

    // Slew rate control OFF (100 kHz standard mode)
    I2C1CONbits.DISSLW = 1;

    // Clear flags
    I2C1STAT = 0;

    // Abilita I2C
    I2C1CONbits.ON = 1;
}

bool i2c_start(void)
{
    if (!i2c_wait_idle()) return false;

    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);

    return true;
}

bool i2c_restart(void)
{
    if (!i2c_wait_idle()) return false;

    I2C1CONbits.RSEN = 1;
    while (I2C1CONbits.RSEN);

    return true;
}

void i2c_stop(void)
{
    if (!i2c_wait_idle()) return;

    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
}

bool i2c_write(uint8_t data)
{
    if (!i2c_wait_idle()) return false;

    I2C1TRN = data;
    while (I2C1STATbits.TRSTAT);

    // ACK ricevuto?
    return (I2C1STATbits.ACKSTAT == 0);
}

uint8_t i2c_read(bool ack)
{
    uint8_t data;

    if (!i2c_wait_idle()) return 0;

    // Abilita ricezione
    I2C1CONbits.RCEN = 1;
    while (!I2C1STATbits.RBF);

    data = I2C1RCV;

    // ACK / NACK
    I2C1CONbits.ACKDT = ack ? 0 : 1;
    I2C1CONbits.ACKEN = 1;
    while (I2C1CONbits.ACKEN);

    return data;
}

// =====================
// Helper
// =====================
static bool i2c_wait_idle(void)
{
    // Attendi che il bus sia idle
    while (I2C1CON & 0x1F);       // SEN, RSEN, PEN, RCEN, ACKEN
    while (I2C1STATbits.TRSTAT); // trasmissione in corso

    return true;
}
