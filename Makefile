# Makefile for building SHA-256 implementation and tests (sha_256 files are in SHA_256/)

CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -I./tests -I./SHA_256 -fsanitize=undefined -O2
LDFLAGS :=

SRC := ./SHA_256/sha_256.c
TEST_SRC := ./tests/test_sha256.c ./tests/parser.c
OUT := ./tests/test.exe

.PHONY: all clean run

all: $(OUT)

$(OUT): $(SRC) $(TEST_SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(TEST_SRC) $(SRC) $(LDFLAGS)

run: $(OUT)
	@echo "Running $(OUT)"
	@if [ -x "$(OUT)" ]; then ./$(OUT); else cmd /c "$(OUT)"; fi

clean:
	@echo "Cleaning build artifacts..."
	@del /Q $(OUT) 2>nul || true
	@rm -f $(OUT) 2>/dev/null || true
