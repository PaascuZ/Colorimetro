/* beep.c
 * PWM 10 kHz duty 50% su OC1 mappato su RB14 (RPB14)
 *
 * - OC1 in PWM mode (Edge-aligned PWM)
 * - Timer2 come timebase
 * - PBCLK = 40 MHz (dal tuo clock standard)
 */

#include "beep.h"

#include <xc.h>
#include <stdint.h>

#include "clock.h"
#include "utils.h"

#define BEEP_FREQ_HZ        10000UL

// Timer2 prescaler 1:8
#define T2_PRESCALE_BITS    0b011
#define T2_PRESCALE_VAL     8UL

// PR2 = (PBCLK / (presc * f_pwm)) - 1
#define PR2_VALUE   ((PBCLK_HZ / (T2_PRESCALE_VAL * BEEP_FREQ_HZ)) - 1UL)

// Duty 50%
#define DUTY_50     ((PR2_VALUE + 1UL) / 2UL)

// PPS: RPB14R = OC1 -> valore 0b1100 (datasheet)
#define PPS_OUT_OC1  0b1100

static void pps_unlock(void)
{
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    CFGCONbits.IOLOCK = 0;
}

static void pps_lock(void)
{
    CFGCONbits.IOLOCK = 1;
    SYSKEY = 0x00000000;
}

static void beep_timer2_init(void)
{
    T2CONbits.ON = 0;                 // OFF while configuring
    T2CONbits.TCKPS = T2_PRESCALE_BITS;
    PR2 = (uint16_t)PR2_VALUE;
    TMR2 = 0;
}

static void beep_oc1_init(void)
{
    OC1CONbits.ON = 0;

    // Timebase = TMR2
    OC1CONbits.OCTSEL = 0;

    // PWM mode
    OC1CONbits.OCM = 0b110;

    // duty 50%
    OC1R  = (uint16_t)DUTY_50;
    OC1RS = (uint16_t)DUTY_50;
}

void beep_init(void)
{
    // RB14 digitale + output
    ANSELBbits.ANSB14 = 0;
    TRISBbits.TRISB14 = 0;
    LATBbits.LATB14 = 0;

    // PPS: OC1 -> RPB14
    pps_unlock();
    RPB14Rbits.RPB14R = PPS_OUT_OC1;
    pps_lock();

    beep_timer2_init();
    beep_oc1_init();

    beep_off();
}

void beep_on(void)
{
    // reset contatore per avere duty pulito
    TMR2 = 0;

    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;
}

void beep_off(void)
{
    OC1CONbits.ON = 0;
    T2CONbits.ON = 0;
    LATBbits.LATB14 = 0;
}

void beep_beep_ms(uint32_t ms)
{
    beep_on();
    utils_delay_ms(ms);
    beep_off();
}
