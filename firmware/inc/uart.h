#include <stdint.h>

#ifndef UART_H
#define UART_H

/**
 * Inizializza UART1
 * - 115200 baud
 * - 8N1
 */
void uart_init(void);

void uart_putc(char c);
void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);

// non-blocking
int  uart_try_getc(char *out);

// blocking (comodo)
char uart_getc_blocking(void);
#endif
