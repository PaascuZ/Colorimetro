/* beep.c
 * PWM 10 kHz duty 50% su OC1 (RB14)
 *
 * Scelta tecnica:
 * - OC1 in PWM mode con Timer2 come timebase
 * - Frequenza PWM = PBCLK / (prescaler * (PR2+1))
 *   con PBCLK=40MHz
 *
 * Obiettivo: 10kHz -> periodo 100 us
 * Scegliamo prescaler 1:8
 *  PR2 = (40MHz/(8*10kHz)) - 1 = 500 - 1 = 499
 * Duty 50% -> OC1RS = (PR2+1)/2 = 250
 */

#include "beep.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "clock.h"
#include "utils.h"

#define BEEP_FREQ_HZ        10000UL
#define T2_PRESCALE_BITS    0b011     // 1:8
#define T2_PRESCALE_VAL     8UL

#define PR2_VALUE   ((PBCLK_HZ / (T2_PRESCALE_VAL * BEEP_FREQ_HZ)) - 1UL)  // 499
#define DUTY_50     ((PR2_VALUE + 1UL) / 2UL)

static void beep_timer2_init(void);
static void beep_oc1_init(void);

void beep_init(void)
{
    TRISBbits.TRISB14 = 0;   // RB14 output

    beep_timer2_init();
    beep_oc1_init();

    beep_off();
}


void beep_on(void)
{
    // Abilita Timer2 e OC1
    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;
}

void beep_off(void)
{
    // Spegni OC1 prima del timer (pulito)
    OC1CONbits.ON = 0;
    T2CONbits.ON = 0;

    // Porta l'uscita "bassa" (evita rimasugli)
    LATBbits.LATB14 = 0;
}

void beep_beep_ms(uint32_t ms)
{
    beep_on();
    utils_delay_ms(ms);
    beep_off();
}

// =====================
// Internals
// =====================
static void beep_timer2_init(void)
{
    // Timer2 OFF per configurazione
    T2CONbits.ON = 0;

    // Prescaler 1:8
    T2CONbits.TCKPS = T2_PRESCALE_BITS;

    // Periodo per 10 kHz
    PR2 = (uint16_t)PR2_VALUE;

    // Reset contatore
    TMR2 = 0;
}

static void beep_oc1_init(void)
{
    OC1CONbits.ON = 0;

    // Usa Timer2 come timebase
    OC1CONbits.OCTSEL = 0;   // TMR2

    // PWM mode
    OC1CONbits.OCM = 0b110;  // Edge-aligned PWM

    // Duty 50%
    OC1R  = (uint16_t)DUTY_50;
    OC1RS = (uint16_t)DUTY_50;
}

