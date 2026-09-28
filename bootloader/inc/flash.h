#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#define ROLLBACK_COUNTER_BASE_ADDR 0x08010000
#define FLASH_INTERFACE_REGISTER_BASE_ADDR 0x40023C00

#define ROLLBACK_SECTOR_BYTES  (64u * 1024u)                          /* sector 4: 64 KB */
#define ROLLBACK_SECTOR_WORDS  (ROLLBACK_SECTOR_BYTES / sizeof(uint32_t))
_Static_assert(ROLLBACK_SECTOR_BYTES == 0x10000, "sector 4 is 64 KB");

#define FLASH_KEYR (*(volatile uint32_t *)(FLASH_INTERFACE_REGISTER_BASE_ADDR + 0x04)) //  FLASH_KEYR Offset = 0x4
#define FLASH_SR (*(volatile uint32_t *)(FLASH_INTERFACE_REGISTER_BASE_ADDR + 0x0C)) // FLASH_SR Offset = 0xC
#define FLASH_CR (*(volatile uint32_t *)(FLASH_INTERFACE_REGISTER_BASE_ADDR + 0x10)) // FLASH_CR Offset = 0x10
#define FLASH_BSY_TIMEOUT 1000000 // Timeout value for flash operations

typedef enum {
    FLASH_SUCCESS = 0,
    NO_INCREMENT_NEEDED = 1,
    VERSION_UPDATE_COUNT_FULL = 2,
    VERSION_MAX_EXCEEDED = 3,
    FLASH_LOCKED = 4,
    FLASH_PROGRAMMING_ERROR = 5,
    FLASH_WRITE_FAILED = 6,
    FLASH_BSY_TIMEOUT_ERROR = 7
}flash_error_status;

uint32_t rollback_get_count(void);           /* scan sector 4 */
int rollback_increment(uint32_t n);     /* write n bytes of 0x00 */

#endif // FLASH_H
