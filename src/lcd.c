#include "lcd.h"
#include "utils.h"
#include <xc.h>

// --- PMP/LCD wiring (Basys MX3) ---
// DISP_RS = RB15 / PMA0
// DISP_RW = RD5  / PMRD
// DISP_EN = RD4  / PMWR
// DB0..DB7 = RE0..RE7 / PMD0..PMD7
// Ref: Basys MX3 RM, LCD connectivity & TRIS/ANSEL hints. 

static inline void lcd_rs_cmd(void) { PMADDR = 0; } // RS=0
static inline void lcd_rs_data(void){ PMADDR = 1; } // RS=1

static void lcd_pins_init(void)
{
    // Command pins as digital outputs
    TRISBbits.TRISB15 = 0; // RS
    ANSELBbits.ANSB15 = 0; // disable analog on RB15 (important)

    TRISDbits.TRISD5 = 0;  // RW (PMRD)
    TRISDbits.TRISD4 = 0;  // EN (PMWR)

    // Data bus RE0..RE7 (PMP drives directions; still set as outputs for write-only use)
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE2 = 0;
    TRISEbits.TRISE3 = 0;
    TRISEbits.TRISE4 = 0;
    TRISEbits.TRISE5 = 0;
    TRISEbits.TRISE6 = 0;
    TRISEbits.TRISE7 = 0;

    // Disable analog on the pins that are multiplexed with ANxx
    // (RM explicitly points out DB2, DB4, DB5, DB6, DB7 have analog functionality) :contentReference[oaicite:3]{index=3}
    ANSELEbits.ANSE2 = 0; // RE2 (DB2)
    ANSELEbits.ANSE4 = 0; // RE4 (DB4)
    ANSELEbits.ANSE5 = 0; // RE5 (DB5)
    ANSELEbits.ANSE6 = 0; // RE6 (DB6)
    ANSELEbits.ANSE7 = 0; // RE7 (DB7)
}

static void lcd_pmp_init(void)
{
    // Disable PMP before config
    PMCONbits.ON = 0;

    // Enable address line A0 (PMA0) -> used as RS
    PMAEN = 0;
    PMAENbits.PTEN0 = 1;

    // Master mode, 8-bit data, no addr multiplexing
    PMMODE = 0;
    PMMODEbits.MODE  = 2;   // Master mode 1 (commonly used for LCD-like devices)
    PMMODEbits.WAITB = 3;   // wait before strobe
    PMMODEbits.WAITM = 15;  // strobe width
    PMMODEbits.WAITE = 3;   // wait after strobe

    // PMCON: enable read/write strobes (even if we only write now)
    PMCON = 0;
    PMCONbits.PTWREN = 1;   // PMWR enabled
    PMCONbits.PTRDEN = 1;   // PMRD enabled
    PMCONbits.ADRMUX = 0;   // address lines not multiplexed
    PMCONbits.PMPEN  = 1;   // enable PMP pins

    // Turn PMP on
    PMCONbits.ON = 1;

    // Default PMADDR (RS=0)
    PMADDR = 0;
}

static void lcd_write_cmd(uint8_t cmd)
{
    lcd_rs_cmd();
    PMDIN = cmd;            // PMP handles the EN strobe timing
    utils_delay_ms(2);            // safe delay for most cmds
}

static void lcd_write_data(uint8_t data)
{
    lcd_rs_data();
    PMDIN = data;
    utils_delay_ms(1);
}

// --- Public API ---

void lcd_init(void)
{
    lcd_pins_init();
    lcd_pmp_init();

    // LCD power-up init sequence (HD44780/KS0066U compatible)
    // Use delays instead of busy-flag: robust and simple.
    utils_delay_ms(20);

    lcd_write_cmd(0x38); // Function set: 8-bit, 2 lines, 5x8
    utils_delay_ms(5);
    lcd_write_cmd(0x38);
    utils_delay_ms(1);
    lcd_write_cmd(0x38);

    lcd_write_cmd(0x0C); // Display ON, cursor OFF, blink OFF
    lcd_clear();
    lcd_write_cmd(0x06); // Entry mode: increment, no shift
    lcd_home();
}

void lcd_clear(void)
{
    lcd_write_cmd(0x01); // Clear display
    utils_delay_ms(5);         // clear needs longer
}

void lcd_home(void)
{
    lcd_write_cmd(0x02); // Return home
    utils_delay_ms(3);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    if (row >= LCD_ROWS) row = 0;
    if (col >= LCD_COLS) col = 0;

    // DDRAM: row0 base 0x00, row1 base 0x40 (per RM) :contentReference[oaicite:4]{index=4}
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    addr += col;

    lcd_write_cmd(0x80 | addr);
}

void lcd_putc(char c)
{
    lcd_write_data((uint8_t)c);
}

void lcd_puts(const char *s)
{
    while (s && *s) {
        lcd_putc(*s++);
    }
}

void lcd_print_line(uint8_t row, const char *s)
{
    lcd_set_cursor(row, 0);

    // stampa max 16 caratteri; se stringa più corta, riempi spazi
    for (uint8_t i = 0; i < LCD_COLS; i++) {
        char c = (s && s[i]) ? s[i] : ' ';
        lcd_putc(c);
        if (s && !s[i]) {
            // finita: riempiamo con spazi fino a 16
            s = NULL;
        }
    }
}
