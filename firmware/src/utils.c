#include "utils.h"

#include <xc.h>
#include <stdint.h>

//#include "config_bits.h"
#include "utils.h"
#include "clock.h"

// =====================
// Costanti Core Timer
// =====================
// Core Timer incrementa a SYSCLK/2
// Con SYSCLK = 80 MHz ? CoreTimer = 40 MHz
#define CORE_TIMER_HZ   (SYSCLK_HZ / 2UL)
#define CORE_TICKS_MS   (CORE_TIMER_HZ / 1000UL)

// =====================
// Stato
// =====================
static volatile uint32_t boot_ms = 0;

// =====================
// API
// =====================
void utils_init(void)
{
    // Reset core timer
    _CP0_SET_COUNT(0);

    // Reset millis counter
    boot_ms = 0;
}

uint32_t utils_millis(void)
{
    // Lettura core timer
    uint32_t ticks = _CP0_GET_COUNT();

    // Conversione tick -> ms
    return ticks / CORE_TICKS_MS;
}

void utils_delay_ms(uint32_t ms)
{
    const uint32_t start = _CP0_GET_COUNT();
    const uint32_t wait_ticks = ms * CORE_TICKS_MS;

    while ((_CP0_GET_COUNT() - start) < wait_ticks) {
        // busy wait
    }
}
