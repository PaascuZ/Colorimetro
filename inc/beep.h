/* beep.h
 * Beep PWM su OC1 (RB14) - Basys MX3 / PIC32MX370
 * Richiesta consegna: 10 kHz, duty 50%
 */
#ifndef BEEP_H
#define BEEP_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Inizializza OC1 su RB14 e il timer per PWM.
 * Non abilita automaticamente l'uscita (beep off).
 */
void beep_init(void);

/**
 * Avvia il beep (PWM attivo).
 */
void beep_on(void);

/**
 * Ferma il beep (PWM disattivo).
 */
void beep_off(void);

/**
 * Beep per una durata in ms (bloccante).
 * Richiede utils_delay_ms().
 */
void beep_beep_ms(uint32_t ms);

#endif // BEEP_H
