#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Inizializza I2C1 come master
 * Clock standard: 100 kHz
 */
void i2c_init(void);

/**
 * Start condition
 */
bool i2c_start(void);

/**
 * Repeated start
 */
bool i2c_restart(void);

/**
 * Stop condition
 */
void i2c_stop(void);

/**
 * Invia un byte
 * return true se ACK ricevuto
 */
bool i2c_write(uint8_t data);

/**
 * Legge un byte
 * ack = true  -> ACK
 * ack = false -> NACK
 */
uint8_t i2c_read(bool ack);

#endif // I2C_H
