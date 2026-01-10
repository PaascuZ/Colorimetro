#ifndef CONFIG_BITS_H
#define CONFIG_BITS_H

#include <xc.h>

/*
 * Basys MX3 (PIC32MX370F512L) - Clock standard scelto:
 * - Sorgente: Oscillatore esterno (Primary Oscillator) 8 MHz
 * - PLL attivo -> SYSCLK = 80 MHz
 * - Peripheral Bus Clock -> PBCLK = 40 MHz
 *
 * PLL setup (tipico per 8 MHz -> 80 MHz):
 *   FPLLICLK = 8 MHz / 2 = 4 MHz
 *   PLLMULT  = 4 MHz * 20 = 80 MHz
 *   PLLODIV  = /1 = 80 MHz
 */

// =====================
// Oscillator / Clocking
// =====================
#pragma config FNOSC    = PRIPLL   // Oscillator Selection: Primary Oscillator with PLL
#pragma config POSCMOD  = HS       // Primary Oscillator Mode: HS (high speed crystal/clock)
#pragma config FPLLIDIV = DIV_2    // PLL Input Divider: 8MHz / 2 = 4MHz
#pragma config FPLLMUL  = MUL_20   // PLL Multiplier: 4MHz * 20 = 80MHz
#pragma config FPLLODIV = DIV_1    // PLL Output Divider: 80MHz / 1 = 80MHz

#pragma config FPBDIV   = DIV_2    // Peripheral Bus Clock: PBCLK = SYSCLK/2 = 40MHz

#pragma config FCKSM    = CSECME   // Clock Switching enabled, FSCM enabled
#pragma config IESO     = OFF      // Two-Speed Start-up disabled

// =====================
// Watchdog / Debug / JTAG
// =====================
#pragma config FWDTEN   = OFF      // Watchdog Timer disabled
#pragma config WDTPS    = PS1048576 // WDT Postscale (irrilevante se FWDTEN=OFF)

#pragma config JTAGEN   = OFF      // JTAG disabled
#pragma config ICESEL   = ICS_PGx1  // ICE/ICD uses PGEC2/PGED2

// =====================
// Misc / Protection
// =====================
#pragma config FSOSCEN  = OFF      // Secondary Oscillator disabled
#pragma config OSCIOFNC = OFF      // CLKO disabled (use as normal I/O)
#pragma config PMDL1WAY = OFF      // Allow multiple reconfig of PMP
#pragma config IOL1WAY  = OFF      // Allow multiple PPS reconfig

// Code protection (OFF)
#pragma config CP       = OFF
#pragma config BWP      = OFF
#pragma config PWP      = OFF

// =====================
// Frequency macros
// =====================
#define SYSCLK_HZ   80000000UL
#define PBCLK_HZ    40000000UL

#endif // CONFIG_BITS_H
