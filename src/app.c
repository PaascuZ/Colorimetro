#include "app.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "uart.h"
#include "i2c.h"
#include "tcs34725.h"
#include "beep.h"
#include "board.h"
// #include "lcd.h"
#include "flash.h"
#include "led.h"

typedef enum {
    APP_STATE_MENU = 0,
    APP_STATE_SCAN,
    APP_STATE_SHOW_COUNT,
    APP_STATE_RESET_FLASH
} app_state_t;

typedef struct {
    app_state_t state;

    // scan
    uint32_t last_read_ms;
    uint32_t scan_start_ms;
    uint32_t red_count;

    // sensor
    bool sensor_ok;
    tcs34725_raw_t raw;

    // menu
    bool menu_printed;
    
    // blink
    uint32_t blink_last_ms;
    uint32_t blink_remaining_toggles;
    bool     blinking;
    uint32_t saved_count;
} app_ctx_t;

static app_ctx_t g_app;

// =====================
// Config
// =====================
#define APP_READ_PERIOD_MS  200U
#define APP_FLASH_ADDR_RED_COUNT   0x000000u   // inizio flash, settore 4KB


// =====================
// Prototipi locali
// =====================
static void app_print_menu(void);
static int  app_uart_try_getc(char *out);     // non-blocking
static void app_handle_menu_choice(char c);

static void app_state_menu_task(void);
static void app_state_scan_task(void);
static void app_state_show_count_task(void);
static void app_state_reset_flash_task(void);

// classificazione minima (per contare i rossi)
static bool app_is_red(const tcs34725_raw_t *s);

// =====================
// API
// =====================
void app_init(void)
{
    // init base
    g_app.state = APP_STATE_MENU;
    g_app.last_read_ms = 0;
    g_app.scan_start_ms = 0;
    g_app.red_count = 0;
    g_app.sensor_ok = false;
    g_app.menu_printed = false;
    
    // driver init
    uart_init();
    uart_puts("\r\n[APP] Boot\r\n");

    i2c_init();
    
    flash_init();
    uart_puts("[APP] FLASH OK\r\n");
    uart_printf("[APP] FLASH SR=0x%02X\r\n", flash_read_status());

    led_init();

    // beep init (non beepiamo qui per non rompere il debug)
    beep_init();

    // sensore
    g_app.sensor_ok = tcs34725_init();
    if (g_app.sensor_ok) {
        tcs34725_set_integration_time(TCS34725_IT_24MS);
        tcs34725_set_gain(TCS34725_GAIN_1X);
        tcs34725_enable(true);
        uart_puts("[APP] TCS34725 OK\r\n");
    } else {
        uart_puts("[APP][ERR] TCS34725 FAIL\r\n");
    }

    g_app.last_read_ms = utils_millis();
}

void app_task(void)
{
    switch (g_app.state) {
        case APP_STATE_MENU:        app_state_menu_task(); break;
        case APP_STATE_SCAN:        app_state_scan_task(); break;
        case APP_STATE_SHOW_COUNT:  app_state_show_count_task(); break;
        case APP_STATE_RESET_FLASH: app_state_reset_flash_task(); break;
        default:
            g_app.state = APP_STATE_MENU;
            g_app.menu_printed = false;
            break;
    }
}

// =====================
// MENU
// =====================
static void app_print_menu(void)
{
    uart_puts("\r\n========================\r\n");
    uart_puts(" COLORIMETER - MENU\r\n");
    uart_puts("========================\r\n");
    uart_puts("1) Start scan\r\n");
    uart_puts("2) Show RED count\r\n");
    uart_puts("3) Reset saved data\r\n");
    uart_puts("------------------------\r\n");
    uart_puts("Select: ");
}

static int app_uart_try_getc(char *out)
{
    // UART4: URXDA = data available
    if (U4STAbits.URXDA) {
        *out = (char)U4RXREG;
        return 1;
    }
    return 0;
}

static void app_handle_menu_choice(char c)
{
    // echo (comodo su PuTTY)
    uart_putc(c);
    uart_puts("\r\n");

    switch (c) {
        case '1':
            g_app.state = APP_STATE_SCAN;
            g_app.scan_start_ms = utils_millis();
            g_app.last_read_ms = g_app.scan_start_ms;
            g_app.red_count = 0;

            uart_puts("[SCAN] Starting...\r\n");
            // beep inizio scansione (breve)
            beep_beep_ms(150);

            break;

        case '2':
            g_app.state = APP_STATE_SHOW_COUNT;
            break;

        case '3':
            g_app.state = APP_STATE_RESET_FLASH;
            break;

        default:
            uart_puts("[MENU] Invalid choice. Press 1,2,3\r\n");
            break;
    }

    // Quando cambio stato, il menu va ristampato al ritorno
    g_app.menu_printed = false;
}

static void app_state_menu_task(void)
{
    g_app.blinking = false;
    led_red_off();
    
    if (!g_app.menu_printed) {
        app_print_menu();
        g_app.menu_printed = true;
    }

    char c;
    if (app_uart_try_getc(&c)) {
        app_handle_menu_choice(c);
    }
}

// =====================
// STATE: SCAN
// =====================
static void app_state_scan_task(void)
{
    if (board_int4_btnc_fired()) {
        board_int4_btnc_clear();

        uart_printf(
            "\r\n[SCAN] Stopped by BTNC. RED count=%lu\r\n",
            (unsigned long)g_app.red_count
        );

        // Salva in flash (erase settore 4KB + write u32)
        uart_puts("[SCAN] Saving to FLASH...\r\n");

        if (!flash_erase_sector_4k(APP_FLASH_ADDR_RED_COUNT)) {
            uart_puts("[SCAN][ERR] FLASH erase failed\r\n");
        } else if (!flash_write_u32(APP_FLASH_ADDR_RED_COUNT, (uint32_t)g_app.red_count)) {
            uart_puts("[SCAN][ERR] FLASH write failed\r\n");
        } else {
            uart_puts("[SCAN] Saved.\r\n");
        }

        g_app.state = APP_STATE_MENU;
        g_app.menu_printed = false;
        return;
    }

    if (!g_app.sensor_ok) {
        uart_puts("[SCAN][ERR] Sensor not available\r\n");
        g_app.state = APP_STATE_MENU;
        return;
    }

    const uint32_t now = utils_millis();

    if ((now - g_app.last_read_ms) >= APP_READ_PERIOD_MS) {
        g_app.last_read_ms = now;

        if (tcs34725_read_raw(&g_app.raw)) {
            // stampa base (debug)
            uart_printf("C=%5u R=%5u G=%5u B=%5u\r\n",
                        (unsigned)g_app.raw.c,
                        (unsigned)g_app.raw.r,
                        (unsigned)g_app.raw.g,
                        (unsigned)g_app.raw.b);

            // Conta rossi (placeholder logico)
            if (app_is_red(&g_app.raw)) {
                g_app.red_count++;
            }

            // In futuro: aggiorna LCD con R/G/B
        } else {
            uart_puts("[SCAN][ERR] Read failed\r\n");
        }
    }

    // Per ora: esci dalla scansione se l'utente preme 'q'
    // (comodo finché INT4 non è implementato)
    char c;
    if (app_uart_try_getc(&c)) {
        if (c == 'q' || c == 'Q') {
            uart_printf("[SCAN] Stop. RED count=%lu\r\n", (unsigned long)g_app.red_count);

            // In futuro: salva g_app.red_count in flash qui (su evento INT4)
            // flash_write_red_count(g_app.red_count);

            g_app.state = APP_STATE_MENU;
            g_app.menu_printed = false;
        }
    }
}

// =====================
// STATE: SHOW COUNT
// =====================
static void app_state_show_count_task(void)
{
    // Prima entrata nello stato
    if (!g_app.blinking) {
        uint32_t saved = 0;

        if (!flash_read_u32(APP_FLASH_ADDR_RED_COUNT, &saved)) {
            uart_puts("\r\n[COUNT][ERR] FLASH read failed\r\n");
            g_app.state = APP_STATE_MENU;
            g_app.menu_printed = false;
            return;
        }

        if (saved == 0xFFFFFFFFu) saved = 0;

        g_app.saved_count = saved;
        uart_printf("\r\n[COUNT] RED count (FLASH) = %lu\r\n", (unsigned long)saved);

        // N volte = N blink completi = 2*N toggle (on/off)
        g_app.blink_remaining_toggles = (uint32_t)(2u * saved);
        g_app.blink_last_ms = utils_millis();
        g_app.blinking = true;

        led_red_off();

        // Se N=0, ritorna subito al menu
        if (g_app.blink_remaining_toggles == 0) {
            g_app.blinking = false;
            g_app.state = APP_STATE_MENU;
            g_app.menu_printed = false;
            return;
        }
    }

    // Lampeggio: toggle ogni 500ms
    uint32_t now = utils_millis();
    if ((now - g_app.blink_last_ms) >= 500u) {
        g_app.blink_last_ms = now;

        led_red_toggle();
        g_app.blink_remaining_toggles--;

        if (g_app.blink_remaining_toggles == 0) {
            led_red_off();
            g_app.blinking = false;
            uart_puts("[COUNT] Blink done.\r\n");

            g_app.state = APP_STATE_MENU;
            g_app.menu_printed = false;
        }
    }
}


// =====================
// STATE: RESET FLASH
// =====================
static void app_state_reset_flash_task(void)
{
    uart_puts("\r\n[RESET] Erasing FLASH sector...\r\n");

    if (!flash_erase_sector_4k(APP_FLASH_ADDR_RED_COUNT)) {
        uart_puts("[RESET][ERR] FLASH erase failed\r\n");
    } else {
        uart_puts("[RESET] Done.\r\n");
    }

    g_app.state = APP_STATE_MENU;
    g_app.menu_printed = false;
}

// =====================
// Classificazione MINIMA per contare i rossi
// (poi la miglioriamo, ma qui serve solo per il contatore)
// =====================
static bool app_is_red(const tcs34725_raw_t *s)
{
    if (s->c < 50) return false;

    // semplice: R dominante con margine
    return (s->r > (uint16_t)(s->g + 40)) && (s->r > (uint16_t)(s->b + 40));
}
