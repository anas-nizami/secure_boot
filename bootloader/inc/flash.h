#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
uint32_t rollback_get_count(void);           /* scan sector 4 */
int      rollback_increment(uint32_t n);     /* write n bytes of 0x00 */

#endif // FLASH_H