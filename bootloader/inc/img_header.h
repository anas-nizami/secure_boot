#ifndef IMG_HEADER_H
#define IMG_HEADER_H

#include <stdint.h>

#define IMG_MAGIC       0x4E495A41u        /* "AZIN" little-endian. This is here to ensure the image is present */
#define IMG_HEADER_SIZE 512u
#define APP_HEADER_ADDR 0x08020000u
#define APP_BODY_ADDR   0x08020200u
#define APP_SLOT_SIZE   (384u * 1024u)

typedef struct {
    uint32_t magic;
    uint32_t version;       /* for Phase 4 rollback */
    uint32_t img_len;       /* body only, header excluded */
    uint32_t reserved;
    uint8_t  hash[32];      /* SHA-256 over the body */
    uint8_t  sig[64];       /* zeros until Phase 3 */
    uint8_t  pad[400];
} img_header_t;

_Static_assert(sizeof(img_header_t) == IMG_HEADER_SIZE,
               "image header must be exactly 512 bytes");

#endif