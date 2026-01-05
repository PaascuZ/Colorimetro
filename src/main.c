/**
 * main.c - Progetto Colorimetro (Basys MX3 + TCS34725)
 *
 * STRUTTURA COERENTE CON IL TUO PROGETTO:
 *  - config_bits.h   : config bits + SYSCLK_HZ / PBCLK_HZ
 *  - board.[ch]      : init board (GPIO/interrupt/JTAG off ecc.)
 *  - utils.[ch]      : timing (delay, millis) + utility base
 *  - app.[ch]        : logica applicazione (init + task loop)
 *
 * NOTE:
 *  - main.c NON include direttamente uart/i2c/tcs/lcd/... perché sono gestiti da app.c
 *  - main.c fa SOLO boot + loop chiamando app_task()
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "config_bits.h"
#include "board.h"
#include "utils.h"
#include "app.h"

// INCLUDE PER TEST
#include "uart.h"
#include "lcd.h"

int main(void)
{
    // Inizializzazione base board (clock già configurato via config_bits.h)
    board_init();

    utils_init();
    
    utils_delay_ms(500);   // <-- SAFE BOOT per debug
    
    // Inizializzazione applicazione (UART/I2C/TCS/LCD/BEEP/FLASH dentro app_init)
    app_init();

    // test uart
    //uart_init();
    
    // test uart funziona
    //uart_puts("HELLO\r\n");
    
    // test lcd
    lcd_init();
    lcd_print_line(0, "Colorimetro");
    lcd_print_line(1, "READY");
    
    // 4) Superloop
    while (1) {
        
        app_task();
        // Per evitare busy-loop:
        // utils_delay_ms(1);
    }

    // Mai raggiunto
    // return 0;
}
