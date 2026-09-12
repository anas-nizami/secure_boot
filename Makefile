# Makefile for building SHA-256 implementation and tests (sha_256 files are in SHA_256/)
# This MakeFile will not run or compile the boot loader, it is only for testing the SHA-256 implementation.

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -I./tests -I./bootloader/SHA_256
CFLAGS_ECC := -Wall -Wextra -std=c11 -Wno-unused-parameter \
              -I./tests -I./bootloader/third_party/micro-ecc -I./bootloader/inc
SAN    := -fsanitize=undefined,address
LDFLAGS :=

SRC := ./bootloader/SHA_256/sha_256.c
SHA_TEST_SRC:= ./tests/test_sha256.c ./tests/parser.c
MICRO_ECC_SRC := ./bootloader/third_party/micro-ecc/uECC.c
MICRO_ECC_TEST := ./tests/test_ecc.c

OUT := ./tests/test_sha256.exe
OUT_ECC := ./tests/test_ecc.exe

.PHONY: all clean run

all: $(OUT) $(OUT_ECC)

$(OUT): $(SRC) $(SHA_TEST_SRC)
	$(CC) $(CFLAGS) $(SAN) -o $(OUT) $(SHA_TEST_SRC) $(SRC) $(LDFLAGS)

$(OUT_ECC): $(MICRO_ECC_SRC) $(MICRO_ECC_TEST)
	$(CC) $(CFLAGS_ECC) $(SAN) -o $(OUT_ECC) $(MICRO_ECC_TEST) $(MICRO_ECC_SRC) $(LDFLAGS)

run_sha256: $(OUT)
	@echo "Running $(OUT)"
	./$(OUT)

run_ecc: $(OUT_ECC)
	@echo "Running $(OUT_ECC)"
	./$(OUT_ECC)

clean:
	@rm -f $(OUT)
	@rm -f $(OUT_ECC)
