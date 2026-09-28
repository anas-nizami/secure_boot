#include <flash.h>
#include <stdio.h>

static const volatile uint32_t *rollback_counter_addr = (const volatile uint32_t *)ROLLBACK_COUNTER_BASE_ADDR;
#define MAX_VERSION_UPDATE_COUNT 256

uint32_t rollback_get_count(void) {

    uint32_t count = 0;
    for (size_t i = 0; i < ROLLBACK_SECTOR_WORDS; i++) {
        
        volatile uint32_t address_value = rollback_counter_addr[i];
        
        if (address_value == 0x00) {
            count += 4;
        }else {
            for (int j = 0; j < 4; j++) 
            {
                if ((address_value & 0xFF) == 0x00) {
                    count++;
                }else {
                    return count;
                }
                address_value >>= 8;
            }
        }
    }
    return count;
}

static int flash_wait_bsy(void) {
    uint32_t spins = FLASH_BSY_TIMEOUT;
    
    while (FLASH_SR & (1u << 16)) {
        if (--spins == 0) return FLASH_BSY_TIMEOUT_ERROR;
    }
    return FLASH_SUCCESS;
}

int rollback_increment(uint32_t n) {
    
    if (n == 0) {
        return FLASH_SUCCESS; // No increment needed
    }

    uint32_t current_count = rollback_get_count();
    if (n > MAX_VERSION_UPDATE_COUNT)
    {
        return VERSION_MAX_EXCEEDED; // Exceeds maximum allowed increment in a single call
    } else if (current_count > (ROLLBACK_SECTOR_BYTES - n )) 
    {
        return VERSION_UPDATE_COUNT_FULL; // Exceeds maximum allowed count. Counter is full and cannot accommodate the requested increment.
    }

    if(flash_wait_bsy() == FLASH_BSY_TIMEOUT_ERROR) 
    {
        // Flash is busy, and we have timed out, cannot proceed with programming
        return FLASH_BSY_TIMEOUT_ERROR;
    }

    FLASH_KEYR = 0x45670123; // Unlock key 1
    FLASH_KEYR = 0xCDEF89AB; // Unlock key 2
    
    if(FLASH_CR & (1u<<31)) { // Check if the LOCK bit (LOCK -> 31) is set in the FLASH_CR register
        return FLASH_LOCKED; // Flash is locked, cannot proceed with programming
    }

    const uint32_t clear_error_mask = (1u<<7)       // Clear the PGSERR: Programming sequence error
                             | (1u<<6)       // Clear the PGPERR: Programming parallelism error
                             | (1u<<5)       // Clear the PGAERR: Programming alignment error
                             | (1u<<4)       // Clear the WRPERR: Write protection error
                             | (1u<<1)       // Clear the OPERR: Operation error
                             | (1u<<0);      // Clear the EOP: End of operation
    FLASH_SR = clear_error_mask; // Clear any existing error flags

    FLASH_CR &= ~(0x3u<<8); // Clear the Program size (PSIZE -> 9:8) bits to enable Program Parallelism. 0b00: x8 
    FLASH_CR |= (0x1u<<0);  // Set the Programming (PG -> 0) bit to 1 to enable programming mode.
    
    flash_error_status flash_write_error = FLASH_SUCCESS;
    for(size_t i = 0; i < n; i++) {
        
        volatile uint8_t *target_address = (volatile uint8_t *)(ROLLBACK_COUNTER_BASE_ADDR + current_count + i);
        *target_address = 0x0; // Write 0x00 to increment the rollback counter

        if(flash_wait_bsy() == FLASH_BSY_TIMEOUT_ERROR) 
        {
            flash_write_error = FLASH_BSY_TIMEOUT_ERROR; // Set the error status
            goto exit; // Jump to the exit section
        }

        if (FLASH_SR & ((1u<<7) | (1u<<6) | (1u<<5) | (1u<<4) | (1u<<1))) 
        {
            flash_write_error = FLASH_PROGRAMMING_ERROR; // Set the error status
            goto exit; // Jump to the exit section
        }

        if(*target_address != 0x00) {
            flash_write_error = FLASH_WRITE_FAILED; // Set the error status
            goto exit; // Jump to the exit section
        }

    }

    exit:
        FLASH_CR &= ~(1u<<0); // Clear the Programming (PG -> 0) bit to disable programming mode.
        FLASH_CR |= (1u<<31); // Set the LOCK bit (LOCK -> 31) to lock the flash again after programming
        return flash_write_error; // Return the status of the flash operation

}

