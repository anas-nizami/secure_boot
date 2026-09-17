# Third-party code

Code in this directory is not mine. It is vendored rather than fetched at
build time so the exact version in use is visible and reproducible.

## micro-ecc

- **Source:** <https://github.com/kmackay/micro-ecc>
- **Version:** commit <541b3a7>, retrieved <Sep 11, 2026>
- **License:** BSD-2-Clause (see `micro-ecc/LICENSE.txt`)
- **Used for:** ECDSA-P256 signature verification in the bootloader

### Why a library rather than my own implementation

SHA-256 is implemented from specification in `SHA_256/` because hash functions
can be validated exhaustively against published test vectors.
Signature verification cannot: it has side-channel and input-validation failure modes that passing tests does not surface.
A reviewed implementation is the correct choice.

### Modifications

None. The source is unmodified upstream. Curve selection is configured
externally via compiler defines so that this directory can be updated cleanly:

    -DuECC_SUPPORTS_secp160r1=0
    -DuECC_SUPPORTS_secp192r1=0
    -DuECC_SUPPORTS_secp224r1=0
    -DuECC_SUPPORTS_secp256k1=0

Only secp256r1 (P-256) is compiled in. The others are unused and would be
dead flash in a 64 KB bootloader reservation.

These modifications can be reviewwed in Project Propertied -> C/C++ Build -> Settings -> MCU/MPU GCC Compiler -> Preprocessor -> Defined Symbols (-D)

### Note on randomness

`uECC_verify` requires no random number generator — verification is
deterministic over public values. The device only verifies, never signs, so no
entropy source is needed on target.
