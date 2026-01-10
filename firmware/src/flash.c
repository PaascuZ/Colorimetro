#include "flash.h"

#include <xc.h>
#include <string.h>

#include "utils.h"   // per utils_millis() / delay 

// ======= Config HW Basys MX3 (SPI1) =======
#define FLASH_CS_TRIS   TRISFbits.TRISF8
#define FLASH_CS_LAT    LATFbits.LATF8

static inline void flash_cs_low(void)  { FLASH_CS_LAT = 0; }
static inline void flash_cs_high(void) { FLASH_CS_LAT = 1; }

// SPI clock: PBCLK = 40MHz (dal tuo setup: SYSCLK=80MHz, PBCLK=40MHz)
// Fsck = PBCLK / (2*(BRG+1))
// Con BRG=19 => Fsck ~ 40MHz/(2*20)=1MHz (safe per partire)
#ifndef FLASH_SPI1_BRG
#define FLASH_SPI1_BRG  19u
#endif

static uint8_t spi1_xfer(uint8_t b)
{
    // wait TX buffer empty
    while (SPI1STATbits.SPITBF) {;}
    SPI1BUF = b;
    // wait RX full
    while (!SPI1STATbits.SPIRBF) {;}
    return (uint8_t)SPI1BUF;
}

static void flash_write_enable(void)
{
    flash_cs_low();
    spi1_xfer(FLASH_CMD_WREN);
    flash_cs_high();
}

uint8_t flash_read_status(void)
{
    uint8_t sr;
    flash_cs_low();
    spi1_xfer(FLASH_CMD_RDSR);
    sr = spi1_xfer(0xFF);
    flash_cs_high();
    return sr;
}

bool flash_wait_ready(uint32_t timeout_ms)
{
    if (timeout_ms == 0) {
        while (flash_read_status() & FLASH_SR_WIP) {;}
        return true;
    }

    uint32_t t0 = utils_millis();
    while (flash_read_status() & FLASH_SR_WIP) {
        if ((utils_millis() - t0) >= timeout_ms) return false;
    }
    return true;
}

void flash_init(void)
{
    // --- CE pin as GPIO output, deasserted high ---
    FLASH_CS_TRIS = 0;
    flash_cs_high();

    // --- PPS remap (da Basys MX3 RM) ---
    // RF2 -> SDO1
    // RF7 -> SDI1
    // SCK1 su RF6 (fixed)
    RPF2R = 0x08;  // SDO1 ? RF2
    SDI1R = 0x0F;  // SDI1 ? RF7

    // --- Direction pins ---
    TRISFbits.TRISF2 = 0; // MOSI (SDO1) out
    TRISFbits.TRISF7 = 1; // MISO (SDI1) in
    TRISFbits.TRISF6 = 0; // SCK1 out

    // Nota: su PIC32MX370, i pin F non sono analogici come RBx, quindi niente ANSEL qui.

    // --- SPI1 reset/init ---
    SPI1CON = 0;
    (void)SPI1BUF;                 // clear
    SPI1STATCLR = _SPI1STAT_SPIROV_MASK;

    SPI1BRG = FLASH_SPI1_BRG;

    // SPI mode 0: CKP=0, CKE=1 (common per flash SPI)
    SPI1CONbits.MSTEN = 1; // master
    SPI1CONbits.CKP   = 0;
    SPI1CONbits.CKE   = 1;
    SPI1CONbits.SMP   = 1; // sample end
    SPI1CONbits.MODE16 = 0;
    SPI1CONbits.MODE32 = 0;

    SPI1CONbits.ON = 1;

    // assicurati che non stia già "busy"
    (void)flash_wait_ready(100);
}

static bool flash_addr_ok(uint32_t addr, size_t len)
{
    if (addr >= FLASH_SIZE_BYTES) return false;
    if (len == 0) return true;
    if ((addr + (uint32_t)len) > FLASH_SIZE_BYTES) return false;
    return true;
}

bool flash_read(uint32_t addr, void *dst, size_t len)
{
    if (!flash_addr_ok(addr, len)) return false;
    if (len == 0) return true;

    uint8_t *p = (uint8_t*)dst;

    flash_cs_low();
    spi1_xfer(FLASH_CMD_READ);
    spi1_xfer((uint8_t)(addr >> 16));
    spi1_xfer((uint8_t)(addr >> 8));
    spi1_xfer((uint8_t)(addr >> 0));

    for (size_t i = 0; i < len; i++) {
        p[i] = spi1_xfer(0xFF);
    }
    flash_cs_high();
    return true;
}

bool flash_erase_sector_4k(uint32_t addr)
{
    if (addr >= FLASH_SIZE_BYTES) return false;

    // allinea a 4KB
    uint32_t a = addr & ~(FLASH_SECTOR_SIZE_4K - 1u);

    flash_write_enable();

    flash_cs_low();
    spi1_xfer(FLASH_CMD_SE_4K);
    spi1_xfer((uint8_t)(a >> 16));
    spi1_xfer((uint8_t)(a >> 8));
    spi1_xfer((uint8_t)(a >> 0));
    flash_cs_high();

    // Sector erase può metterci decine/centinaia di ms: timeout conservativo
    return flash_wait_ready(2000);
}

static bool flash_page_program(uint32_t addr, const uint8_t *src, size_t len)
{
    // len MUST be <= 256 and must not cross page boundary
    flash_write_enable();

    flash_cs_low();
    spi1_xfer(FLASH_CMD_PP);
    spi1_xfer((uint8_t)(addr >> 16));
    spi1_xfer((uint8_t)(addr >> 8));
    spi1_xfer((uint8_t)(addr >> 0));

    for (size_t i = 0; i < len; i++) {
        spi1_xfer(src[i]);
    }
    flash_cs_high();

    // Program page tipicamente pochi ms: timeout conservativo
    return flash_wait_ready(200);
}

bool flash_write(uint32_t addr, const void *src, size_t len)
{
    if (!flash_addr_ok(addr, len)) return false;
    if (len == 0) return true;

    const uint8_t *p = (const uint8_t*)src;

    while (len > 0) {
        uint32_t page_off = addr & (FLASH_PAGE_SIZE - 1u);
        uint32_t chunk = FLASH_PAGE_SIZE - page_off;
        if (chunk > len) chunk = (uint32_t)len;

        if (!flash_page_program(addr, p, chunk)) return false;

        addr += chunk;
        p    += chunk;
        len  -= chunk;
    }
    return true;
}

bool flash_chip_erase(void)
{
    flash_write_enable();

    flash_cs_low();
    spi1_xfer(FLASH_CMD_CE);
    flash_cs_high();

    // Chip erase può durare molti secondi: timeout molto largo
    return flash_wait_ready(120000);
}

bool flash_write_u32(uint32_t addr, uint32_t value)
{
    // tipicamente conviene cancellare prima il settore (4KB) dove scrivi
    // qui NON lo facciamo automaticamente per lasciarti controllo dall'app.
    uint8_t b[4];
    b[0] = (uint8_t)(value >> 0);
    b[1] = (uint8_t)(value >> 8);
    b[2] = (uint8_t)(value >> 16);
    b[3] = (uint8_t)(value >> 24);
    return flash_write(addr, b, sizeof(b));
}

bool flash_read_u32(uint32_t addr, uint32_t *value)
{
    uint8_t b[4];
    if (!flash_read(addr, b, sizeof(b))) return false;

    *value =  ((uint32_t)b[0] << 0)
            | ((uint32_t)b[1] << 8)
            | ((uint32_t)b[2] << 16)
            | ((uint32_t)b[3] << 24);
    return true;
}
