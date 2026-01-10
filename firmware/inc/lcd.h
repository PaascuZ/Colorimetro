#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_COLS 16
#define LCD_ROWS 2

void lcd_init(void);

void lcd_clear(void);
void lcd_home(void);

void lcd_set_cursor(uint8_t row, uint8_t col);

void lcd_putc(char c);
void lcd_puts(const char *s);

/**
 * Print one full line (16 chars).
 * If s is shorter, the remaining characters are filled with spaces.
 */
void lcd_print_line(uint8_t row, const char *s);

#endif // LCD_H
