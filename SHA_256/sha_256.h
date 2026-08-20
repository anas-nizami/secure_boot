#ifndef SHA_256_H    // Include guard: checks if SHA_256_H is not defined
#define SHA_256_H    // Defines SHA_256_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t state[8];       // h0..h7
    uint64_t bitlen;         // total message length in BITS
    uint8_t  buffer[64];     // partial block accumulator
    size_t   buflen;         // bytes currently in buffer
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len);
void sha256_final(sha256_ctx *ctx, uint8_t digest[32]);

#endif // SHA_256_H