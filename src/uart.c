#include "uart.h"
#include <xc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "clock.h"

#define UART_BAUDRATE 115200UL
#define UART_BRG_VALUE ((PBCLK_HZ / (16UL * UART_BAUDRATE)) - 1UL)

/*
 * UART4 su Basys MX3:
 *   TX: RF12
 *   RX: RF13
 *
 * PPS (Peripheral Pin Select):
 *   - Mappa U4TX su RPF12
 *   - Mappa U4RX su RPF13
 */
void uart_init(void)
{
    // Direzione pin
    TRISFbits.TRISF12 = 0;   // TX out
    TRISFbits.TRISF13 = 1;   // RX in

    // PPS mapping
    RPF12R = 0b0010;         // U4TX -> RF12
    U4RXR  = 0b0101;         // RF13 -> U4RX

    // Config UART4
    U4MODEbits.ON = 0;

    U4MODEbits.BRGH  = 0;
    U4MODEbits.PDSEL = 0;
    U4MODEbits.STSEL = 0;

    U4BRG = (uint16_t)UART_BRG_VALUE;

    U4STAbits.UTXEN = 1;
    U4STAbits.URXEN = 1;

    U4MODEbits.ON = 1;
}


void uart_putc(char c)
{
    while (U4STAbits.UTXBF);
    U4TXREG = c;
}

void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

void uart_printf(const char *fmt, ...)
{
    char buf[128];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uart_puts(buf);
}
