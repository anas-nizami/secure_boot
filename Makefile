# Makefile for building SHA-256 implementation and tests (sha_256 files are in SHA_256/)
# This MakeFile will not run or compile the boot loader, it is only for testing the SHA-256 implementation.

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -I./tests -I./bootloader/SHA_256
SAN    := -fsanitize=undefined,address
LDFLAGS :=

SRC := ./bootloader/SHA_256/sha_256.c
SHA_TEST_SRC:= ./tests/test_sha256.c ./tests/parser.c
OUT := ./tests/test_sha256.exe

.PHONY: all clean run

all: $(OUT)

$(OUT): $(SRC) $(SHA_TEST_SRC)
	$(CC) $(CFLAGS) $(SAN) -o $(OUT) $(SHA_TEST_SRC) $(SRC) $(LDFLAGS)

run: $(OUT)
	@echo "Running $(OUT)"
	./$(OUT)

clean:
	@rm -f $(OUT)
