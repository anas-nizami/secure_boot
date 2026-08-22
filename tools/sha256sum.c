#include <stdio.h>
#include <sha_256.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc == 2) {
        FILE *f = fopen(argv[1], "rb");
        if (!f) return 1;

        sha256_ctx c;
        uint8_t buf[1024], digest[32];
        size_t n;

        sha256_init(&c);
        while ((n = fread(buf, 1, sizeof buf, f)) > 0)
            sha256_update(&c, buf, n);
        sha256_final(&c, digest);
        fclose(f);

        for (int i = 0; i < 32; i++) printf("%02x", digest[i]);
        printf("\n");
        return 0;
    }
    /* ... existing vector tests ... */
}