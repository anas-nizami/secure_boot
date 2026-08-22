#include <stdio.h>
#include <sha_256.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"

static int hex_to_bytes(const char *hex, uint8_t *out, size_t out_cap, size_t *out_len)
{
    size_t hexlen = strlen(hex);
    size_t i;

    if (hexlen % 2 != 0 || hexlen / 2 > out_cap)
        return 0;

    for (i = 0; i < hexlen; i += 2) {
        unsigned int byte;
        if (sscanf(hex + i, "%2x", &byte) != 1)
            return 0;
        out[i / 2] = (uint8_t)byte;
    }
    *out_len = hexlen / 2;
    return 1;
}

bool test_rsp_vectors(const struct test_vector *vectors, size_t nvectors) {
    bool all_passed = true;
    size_t i;

    for (i = 0; i < nvectors; i++) {
        sha256_ctx ctx;
        uint8_t digest[32];
        uint8_t msg_bytes[64];
        uint8_t expected_digest[32];
        size_t msg_hexlen, digest_len;
        size_t msg_len = (size_t)(vectors[i].length / 8); /* ShortMsg lengths are byte-aligned */

        if (!hex_to_bytes(vectors[i].msg, msg_bytes, sizeof msg_bytes, &msg_hexlen) ||
            !hex_to_bytes(vectors[i].digest, expected_digest, sizeof expected_digest, &digest_len)) {
            printf("Vector %zu (L=%d): invalid hex data\n", i, vectors[i].length);
            all_passed = false;
            continue;
        }

        sha256_init(&ctx);
        sha256_update(&ctx, msg_bytes, msg_len);
        sha256_final(&ctx, digest);

        printf("Testing RSP vector %zu (L=%d): ", i, vectors[i].length);
        if (memcmp(digest, expected_digest, sizeof digest) != 0) {
            printf("\033[1;31mFAILED\033[0m\n");
            all_passed = false;
        } else {
            printf("\033[1;32mPASSED\033[0m\n");
        }
    }

    return all_passed;
}

int main(void) {
    struct test_vector vectors[TV_MAX];
    size_t nvectors;

    nvectors = parse_rsp_vectors("tests/SHA256ShortMsg.rsp", vectors, TV_MAX);
    if(nvectors == 0) {
        printf("No RSP test vectors found or failed to parse the file.\n");
        return 1;
    }
    
    printf("Parsed %zu RSP test vectors\n", nvectors);
    if (nvectors > 0 && test_rsp_vectors(vectors, nvectors))
    {
        printf("\033[1;32m");
        printf("**ALL RSP TESTS PASSED!**\n");
        printf("\033[0m");
        return 0;
    }else
    {
        printf("\033[1;31m");
        printf("**ONE OR MORE RSP TESTS FAILED!**\n");
        printf("\033[0m");
        return 1;
    }

    return 0;
}
