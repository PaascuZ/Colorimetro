#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * Inizializza UART1
 * - 115200 baud
 * - 8N1
 */
void uart_init(void);

/**
 * Invia un carattere
 */
void uart_putc(char c);

/**
 * Invia una stringa null-terminated
 */
void uart_puts(const char *s);

/**
 * printf minimale su UART
 */
void uart_printf(const char *fmt, ...);

#endif // UART_H
