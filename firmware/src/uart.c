#include "uart.h"

#include <xc.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "clock.h"

// =====================
// Config
// =====================
#define UART_BAUDRATE   115200UL
#define UART_BRG_VALUE  ((PBCLK_HZ / (16UL * UART_BAUDRATE)) - 1UL)

// Basys MX3 UART4 pins:
//  TX = RF12 (RPF12R = 0x02 -> U4TX)
//  RX = RF13 (U4RXR  = 0x09 -> RPF13)
#define UART4_TX_PPS    0x02u
#define UART4_RX_PPS    0x09u

// =====================
// Internal helpers
// =====================
static inline void uart4_clear_oerr(void)
{
    if (U4STAbits.OERR) {
        // Clear overrun error (otherwise RX can stall)
        U4STACLR = _U4STA_OERR_MASK;
    }
}

// =====================
// Public API
// =====================
void uart_init(void)
{
    // Pin directions
    TRISFbits.TRISF12 = 0; // TX out
    TRISFbits.TRISF13 = 1; // RX in

    // Disable UART during configuration
    U4MODEbits.ON = 0;

    // PPS mapping
    // TX: U4TX -> RF12
    RPF12R = UART4_TX_PPS;

    // RX: RF13 -> U4RX
    // IMPORTANT: for Basys MX3 mapping this is 0x09
    U4RXRbits.U4RXR = UART4_RX_PPS;

    // UART mode: 8N1, standard speed
    U4MODEbits.BRGH  = 0; // Standard speed mode (16x)
    U4MODEbits.PDSEL = 0; // 8-bit, no parity
    U4MODEbits.STSEL = 0; // 1 stop bit

    // Baud rate
    U4BRG = (uint16_t)UART_BRG_VALUE;

    // Clear status errors
    U4STACLR = _U4STA_OERR_MASK;

    // Enable UART module first, then TX/RX (clean sequence)
    U4MODEbits.ON = 1;

    U4STAbits.UTXEN = 1;
    U4STAbits.URXEN = 1;
}

void uart_putc(char c)
{
    // Wait until TX buffer has space
    while (U4STAbits.UTXBF) {;}
    U4TXREG = c;
}

void uart_puts(const char *s)
{
    if (!s) return;
    while (*s) {
        uart_putc(*s++);
    }
}

int uart_try_getc(char *out)
{
    if (!out) return 0;

    uart4_clear_oerr();

    if (U4STAbits.URXDA) {
        *out = (char)U4RXREG;
        return 1;
    }
    return 0;
}

char uart_getc_blocking(void)
{
    char c;
    while (!uart_try_getc(&c)) {;}
    return c;
}

void uart_printf(const char *fmt, ...)
{
    if (!fmt) return;

    char buf[128];
    va_list args;

    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uart_puts(buf);
}
