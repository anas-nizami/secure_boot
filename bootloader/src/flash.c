#include <flash.h>

#define ROLLBACK_COUNTER_BASE_ADDR 0x08010000
#define ROLLBACK_COUNTER_SIZE ((64*124)/sizeof(uint32_t))  /* 64KB sector, 124 bytes used for rollback counter */
static const volatile uint32_t *rollback_counter_addr = (const volatile uint32_t *)ROLLBACK_COUNTER_BASE_ADDR;


uint32_t rollback_get_count(void) {

    uint32_t count = 0;
    for (size_t i = 0; i < ROLLBACK_COUNTER_SIZE; i++) {
        
        if (rollback_counter_addr[i] == 0x00) {
            count += 4;
        }else {
            volatile uint32_t address_value = rollback_counter_addr[i];
            for (int j = 0; j < 4; j++) {
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