/*
 * dump_format.c - prints the compiler-computed layout of img_header_t.
 *
 * This exists purely so tests/test_format_sync.py can check that
 * tools/sign_image.py's hand-written struct.pack('<IIII32s64s', ...) layout
 * still matches bootloader/inc/img_header.h. It reports what the compiler
 * actually laid out (via offsetof/sizeof), not the header's text, so a
 * reorder/resize of a field in img_header_t is caught even though the C
 * side still compiles fine.
 */
#include <stdio.h>
#include <stddef.h>
#include "img_header.h"

int main(void)
{
    printf("MAGIC=0x%08x\n", IMG_MAGIC);
    printf("HEADER_SIZE=%u\n", IMG_HEADER_SIZE);
    printf("OFF_MAGIC=%zu\n",     offsetof(img_header_t, magic));
    printf("OFF_VERSION=%zu\n",  offsetof(img_header_t, version));
    printf("OFF_IMG_LEN=%zu\n",  offsetof(img_header_t, img_len));
    printf("OFF_RESERVED=%zu\n", offsetof(img_header_t, reserved));
    printf("OFF_HASH=%zu\n",     offsetof(img_header_t, hash));
    printf("OFF_SIG=%zu\n",      offsetof(img_header_t, sig));
    printf("SIZE_HASH=%zu\n", sizeof(((img_header_t *)0)->hash));
    printf("SIZE_SIG=%zu\n",  sizeof(((img_header_t *)0)->sig));
    return 0;
}
