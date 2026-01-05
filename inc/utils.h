#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Inizializza le utility di sistema
 * - reset core timer
 * - prepara millis()
 */
void utils_init(void);

/**
 * Ritorna il tempo in millisecondi dal boot
 */
uint32_t utils_millis(void);

/**
 * Delay bloccante in millisecondi
 */
void utils_delay_ms(uint32_t ms);

#endif // UTILS_H
