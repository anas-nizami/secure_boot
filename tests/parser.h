#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define TV_MAX 128 /* SHA256ShortMsg.rsp has 65 (0..512 bits, step 8) */

struct test_vector {
    int length;
    char msg[4096];
    char digest[4096];
};

/* Parses a NIST SHA256ShortMsg.rsp file at `path` into `vectors`
 * (capacity `max_vectors`). Returns the number of vectors parsed,
 * or 0 if the file could not be opened. */
size_t parse_rsp_vectors(const char *path, struct test_vector *vectors, size_t max_vectors);

#endif
