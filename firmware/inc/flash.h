#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Basys MX3 onboard SPI Flash (Spansion S25FL132, 4MB)
 * Address range: 0x000000 .. 0x3FFFFF
 *
 * Connectivity (SPI1):
 *  - CE  = RF8 (GPIO)
 *  - SCK = RF6 (SCK1)
 *  - SI  = RF2 (needs PPS: RPF2R = 0x08 -> SDO1 on RF2)
 *  - SO  = RF7 (needs PPS: SDI1R = 0x0F -> SDI1 on RF7)
 */

#define FLASH_SIZE_BYTES          (4u * 1024u * 1024u)
#define FLASH_SECTOR_SIZE_4K      4096u
#define FLASH_PAGE_SIZE           256u

// Comandi principali (compatibili con S25FL1xx)
#define FLASH_CMD_WREN            0x06u
#define FLASH_CMD_WRDI            0x04u
#define FLASH_CMD_RDSR            0x05u
#define FLASH_CMD_READ            0x03u
#define FLASH_CMD_PP              0x02u
#define FLASH_CMD_SE_4K           0x20u
#define FLASH_CMD_CE              0xC7u  // Chip Erase (alternativa spesso 0x60)

#define FLASH_SR_WIP              0x01u  // Write In Progress

// Inizializza SPI1 + pin CE e PPS mapping.
void flash_init(void);

// Read status register (0x05)
uint8_t flash_read_status(void);

// Attende che WIP=0 (flash pronta). timeout_ms=0 => attende "per sempre".
bool flash_wait_ready(uint32_t timeout_ms);

// Lettura generica
bool flash_read(uint32_t addr, void *dst, size_t len);

// Erase settore 4KB (addr può essere qualunque: viene allineato a 4KB)
bool flash_erase_sector_4k(uint32_t addr);

// Scrittura (gestisce i boundary di pagina da 256B)
bool flash_write(uint32_t addr, const void *src, size_t len);

// Chip erase (lento!)
bool flash_chip_erase(void);

// Helpers comodi per salvare un contatore a un address fissato dall?app
bool flash_write_u32(uint32_t addr, uint32_t value);
bool flash_read_u32(uint32_t addr, uint32_t *value);

#endif // FLASH_H
