/**
 * @file sha_256.c
 * @author Anas Nizami
 * @brief Implementation of the SHA-256 hashing algorithm.
 * @version 0.1
 * @date 2026-08-20
 * 
 * @copyright Copyright (c) 2026
 * 
 * The functions and values used here are based on the SHA-256 algorithm as defined in the FIPS PUB 180-4 standard.
 * Link to the publication (free of charge): https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
 */

#include "sha_256.h"
#include <string.h>

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))

/* uppercase SIGMA — Three rotation, no shifting */
#define SIG0(x)  (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))         //Doc : 4.4
#define SIG1(x)  (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))         //Doc : 4.5

/* lowercase sigma — Two rotation, one shifting */
#define sig0(x)  (ROTR(x,7)  ^ ROTR(x,18) ^ ((x) >> 3))         //Doc : 4.6
#define sig1(x)  (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))        //Doc : 4.7

/* the other two */
#define CH(x,y,z)   (((x) & (y)) ^ (~(x) & (z)))                //Doc : 4.2
#define MAJ(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))   //Doc : 4.3

#define SHA256_MAX_PADDING 72   /* worst case: buflen == 56 -> (120-56) + 8 */

void sha256_init(sha256_ctx *ctx) {
    
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;

    ctx->bitlen = 0;
    ctx->buflen = 0;

    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

/**
 * @brief Const values from the doc. Look for SHA-256 Constants
 * 
 */
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

static void transform(sha256_ctx *ctx, const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;

    // Prepare the message schedule
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4]     << 24)
            |  ((uint32_t)block[i * 4 + 1] << 16)
            |  ((uint32_t)block[i * 4 + 2] <<  8)
            |  (uint32_t)block[i * 4 + 3];
    }
    for (int i = 16; i < 64; i++) {
        w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
    }

    // Initialize working variables with current hash value
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    // Main loop
    for (int i = 0; i < 64; i++) {
        t1 = h + SIG1(e) + CH(e, f, g) + k[i] + w[i];
        t2 = SIG0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    // Add the compressed chunk to the current hash value
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {

    for (size_t i = 0; i < len; i++) 
    {
        ctx->buffer[ctx->buflen++] = data[i];      
        if(ctx->buflen == 64) 
        {
            transform(ctx, ctx->buffer);
            ctx->buflen = 0;
        }
    }
    ctx->bitlen += len * 8; // Update the total message length in bits
}

void sha256_final(sha256_ctx *ctx, uint8_t digest[32]) {
    size_t padding_length = ctx->buflen < 56 ? 56 - ctx->buflen : 120 - ctx->buflen;
    uint8_t padding[ SHA256_MAX_PADDING ];

    memset(padding, 0, sizeof(padding));
    padding[0] = 0x80;

    uint64_t bitlen = ctx->bitlen;
    for (size_t i = 0; i < 8; i++) {
        padding[padding_length + i] = (uint8_t)(bitlen >> (56 - i * 8));
    }

    sha256_update(ctx, padding, padding_length + 8);

    for (size_t i = 0; i < 8; i++) {
        digest[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)ctx->state[i];
    }
}