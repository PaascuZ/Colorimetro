// firmware/inc/lcd.h
#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

// LCD 16x2
#define LCD_COLS 16
#define LCD_ROWS 2

void lcd_init(void);

void lcd_clear(void);
void lcd_home(void);

void lcd_set_cursor(uint8_t row, uint8_t col);

void lcd_putc(char c);
void lcd_puts(const char *s);

// Utility comoda: stampa stringa ?tagliando? o andando a capo
void lcd_print_line(uint8_t row, const char *s);

#endif // LCD_H
