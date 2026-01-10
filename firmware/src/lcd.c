#include "lcd.h"
#include "utils.h"
#include <xc.h>

// Basys MX3 LCD via PMP:
//  RS  -> RB15 -> PMA0
//  RW  -> RD5  -> PMRD (non usato: write-only)
//  EN  -> RD4  -> PMWR
//  D0..D7 -> RE0..RE7 -> PMD0..PMD7

#define LCD_ADDR_CMD   0u   // RS=0
#define LCD_ADDR_DATA  1u   // RS=1

static inline void pmp_wait_ready(void)
{
    while (PMMODEbits.BUSY) {;}
}

static void lcd_write_cmd(uint8_t cmd)
{
    pmp_wait_ready();
    PMADDR = LCD_ADDR_CMD;
    PMDIN  = cmd;

    // Clear/Home need longer time
    if (cmd == 0x01u || cmd == 0x02u) {
        utils_delay_ms(2);
    } else {
        // 37 us typical; 1 ms is safe
        utils_delay_ms(1);
    }
}

static void lcd_write_data(uint8_t data)
{
    pmp_wait_ready();
    PMADDR = LCD_ADDR_DATA;
    PMDIN  = data;

    utils_delay_ms(1);
}

static void lcd_gpio_init(void)
{
    // RS pin must be digital
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;

    // PMP strobes pins
    TRISDbits.TRISD4 = 0; // PMWR (EN)
    TRISDbits.TRISD5 = 0; // PMRD (RW) - unused but keep as output

    // Data bus: all digital
    ANSELE = 0x0000;
    TRISE  = 0x0000;
}

static void lcd_pmp_init(void)
{
    // Clean reset
    PMCON  = 0;
    PMMODE = 0;
    PMAEN  = 0;

    // Enable address line A0 for RS
    PMAENbits.PTEN0 = 1;

    // Master mode timing (safe/slow)
    PMMODEbits.MODE  = 2;   // Master mode
    PMMODEbits.WAITB = 3;
    PMMODEbits.WAITM = 15;
    PMMODEbits.WAITE = 3;

    // Enable PMP, write strobe
    PMCONbits.ADRMUX = 0;
    PMCONbits.PTWREN = 1;   // enable PMWR
    PMCONbits.PTRDEN = 0;   // disable reads (write-only, more stable)
    PMCONbits.PMPEN  = 1;

    PMCONbits.ON = 1;

    utils_delay_ms(30);
}

void lcd_init(void)
{
    lcd_gpio_init();
    lcd_pmp_init();

    // Init sequence for 8-bit, 2 lines
    lcd_write_cmd(0x38); // function set
    utils_delay_ms(5);
    lcd_write_cmd(0x38);
    utils_delay_ms(1);
    lcd_write_cmd(0x38);

    lcd_write_cmd(0x0C); // display on, cursor off
    lcd_write_cmd(0x06); // entry mode
    lcd_write_cmd(0x01); // clear
    utils_delay_ms(2);
}

void lcd_clear(void)
{
    lcd_write_cmd(0x01);
}

void lcd_home(void)
{
    lcd_write_cmd(0x02);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    if (row >= LCD_ROWS) row = 0;
    if (col >= LCD_COLS) col = 0;

    uint8_t addr = (row == 0) ? 0x00u : 0x40u;
    addr = (uint8_t)(addr + col);

    lcd_write_cmd((uint8_t)(0x80u | addr));
}

void lcd_putc(char c)
{
    lcd_write_data((uint8_t)c);
}

void lcd_puts(const char *s)
{
    if (!s) return;
    while (*s) {
        lcd_putc(*s++);
    }
}

void lcd_print_line(uint8_t row, const char *s)
{
    lcd_set_cursor(row, 0);

    for (uint8_t i = 0; i < LCD_COLS; i++) {
        char c = ' ';
        if (s && s[i]) c = s[i];
        lcd_putc(c);
    }
}
