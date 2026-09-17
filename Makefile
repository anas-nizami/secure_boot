# Makefile for building SHA-256 implementation and tests (sha_256 files are in SHA_256/)
# This MakeFile will not run or compile the boot loader, it is only for testing the SHA-256 implementation.

CC := gcc
PYTHON := python3
CFLAGS_COMMON := -Wall -Wextra -std=c11
CFLAGS := $(CFLAGS_COMMON) -Werror -I./tests -I./bootloader/SHA_256
CFLAGS_ECC := $(CFLAGS_COMMON) -Wno-unused-parameter \
              -I./tests -I./bootloader/third_party/micro-ecc -I./bootloader/inc
CFLAGS_FMT := $(CFLAGS_COMMON) -Werror -I./bootloader/inc
SAN    := -fsanitize=undefined,address
LDFLAGS :=

SHA_SRC := ./bootloader/SHA_256/sha_256.c
SHA_TEST_SRC:= ./tests/SHA_Test/test_sha256.c ./tests/SHA_Test/parser.c
MICRO_ECC_SRC := ./bootloader/third_party/micro-ecc/uECC.c
MICRO_ECC_TEST := ./tests/ECC_Test/test_ecc.c
DUMP_FORMAT_SRC := ./tests/Format_Test/dump_format.c

OUT := ./tests/SHA_Test/test_sha256.exe
OUT_ECC := ./tests/ECC_Test/test_ecc.exe
OUT_FMT := ./tests/Format_Test/dump_format.exe

.PHONY: all run_sha256 run_ecc run_format_sync delete clear test

all: $(OUT) $(OUT_ECC) $(OUT_FMT)

$(OUT): $(SHA_SRC) $(SHA_TEST_SRC)
	$(CC) $(CFLAGS) $(SAN) -o $(OUT) $(SHA_TEST_SRC) $(SHA_SRC) $(LDFLAGS)

$(OUT_ECC): $(MICRO_ECC_SRC) $(MICRO_ECC_TEST)
	$(CC) $(CFLAGS_ECC) $(SAN) -o $(OUT_ECC) $(MICRO_ECC_TEST) $(MICRO_ECC_SRC) $(LDFLAGS)

$(OUT_FMT): $(DUMP_FORMAT_SRC)
	$(CC) $(CFLAGS_FMT) -o $(OUT_FMT) $(DUMP_FORMAT_SRC) $(LDFLAGS)

run_sha256: $(OUT)
	@echo "Running $(OUT)"
	./$(OUT)

run_ecc: $(OUT_ECC)
	@echo "Running $(OUT_ECC)"
	./$(OUT_ECC)

run_format_sync: $(OUT_FMT)
	@echo "Running tests/Format_Test/test_format_sync.py"
	$(PYTHON) ./tests/Format_Test/test_format_sync.py

test: run_sha256 run_ecc run_format_sync

delete:
	@rm -f $(OUT) $(OUT_ECC) $(OUT_FMT)

clear: delete all
	@echo "Rebuilt $(OUT), $(OUT_ECC), $(OUT_FMT)"
