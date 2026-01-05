#include "led.h"
#include <xc.h>

#define LEDR_TRIS   TRISBbits.TRISB7
#define LEDR_LAT    LATBbits.LATB7

void led_init(void)
{
    LEDR_TRIS = TRISBbits.TRISB7; //outpu
    LEDR_LAT  = LATBbits.LATB7; //off
}

void led_red_on(void)    { LEDR_LAT = 1; }
void led_red_off(void)   { LEDR_LAT = 0; }
void led_red_toggle(void){ LEDR_LAT ^= 1; }
