# STM32F407 Secure Boot

A bare-metal secure bootloader for the STM32F407 (Cortex-M4) that verifies an
ECDSA-P256 signature over a SHA-256 image hash before executing application firmware,
and refuses to boot tampered or unsigned images.

> **Status:** in development. See [Roadmap](#roadmap).

![CI](https://github.com/anas-nizami/secure_boot/actions/workflows/ci.yml/badge.svg)

## Why

Firmware running without verification means anyone who can write flash can run code with
full hardware privilege. Secure boot establishes that only firmware signed by the holder
of a private key will execute — the foundation every other embedded security control
rests on.

## Design

```text
POWER ON / RESET
      │
      ▼
┌─────────────────────────────────────┐
│ ST mask ROM  (not yours, unchanged) │
│ BOOT0 low → jump to 0x08000000      │
└─────────────────────────────────────┘
      │
      ▼
╔═════════════════════════════════════════════════╗
║ YOUR BOOTLOADER  @ 0x08000000  (sectors 0–3)    ║
╠═════════════════════════════════════════════════╣
║                                                 ║
║  init clocks, GPIO (LEDs)                       ║
║           │                                     ║
║           ▼                                     ║
║  read header @ 0x08020000                       ║
║           │                                     ║
║           ▼                                     ║
║     magic == 0x4E495A41 ? ──── no ──┐           ║
║           │ yes                     │           ║
║           ▼                         │           ║
║     img_len sane ? ────────── no ───┤           ║
║           │ yes                     │           ║
║           ▼                         │           ║
║  ┌──────────────────────────┐       │           ║
║  │ SHA-256 over body        │       │  PHASE 2  ║
║  │ 0x08020200 .. +img_len   │       │           ║
║  └──────────────────────────┘       │           ║
║           │                         │           ║
║           ▼                         │           ║
║     hash == header.hash ? ─── no ───┤           ║
║           │ yes                     │           ║
║           ▼                         │           ║
║  ┌──────────────────────────┐       │           ║
║  │ ECDSA-P256 verify        │       │  PHASE 3  ║
║  │ sig over hash, pubkey    │       │           ║
║  └──────────────────────────┘       │           ║
║           │                         │           ║
║           ▼                         │           ║
║     signature valid ? ─────── no ───┤           ║
║           │ yes                     │           ║
║           ▼                         │           ║
║     version >= counter ? ──── no ───┤  PHASE 4  ║
║           │ yes                     │           ║
║           ▼                         ▼           ║
║      JUMP TO APP                REFUSE          ║
║           │                    red LED          ║
║           │                    halt forever     ║
╚═══════════│═════════════════════════════════════╝
            ▼
┌─────────────────────────────────────┐
│ APPLICATION @ 0x08020200            │
│ green LED, runs normally            │
└─────────────────────────────────────┘
```

### Flash map (STM32F407VG, 1 MB)

| Sector | Address | Size | Use |
|---|---|---|---|
| 0-3 | 0x08000000 | 16 KB each | Bootloader |
| 4 | 0x08010000 | 64 KB | Metadata / rollback counter |
| 5-7 | 0x08020000 | 128 KB each | App slot A |
| 8-10 | 0x08080000 | 128 KB each | App slot B (update staging) |

Sector sizes on this part are non-uniform, and erase granularity is one sector — this
drives the partitioning.

## Threat model

See [`docs/THREAT_MODEL.md`](docs/THREAT_MODEL.md). Read it before the code: it states
what this design defends against and, more importantly, what it does not.

**Key limitation:** the F407 has no immutable ROM root of trust and no TrustZone. The
bootloader is the trust anchor by *assumption*, protected by flash write protection and
readout protection rather than by hardware. On an STM32H5/U5, ROM would verify the
bootloader itself.

## Cryptography

| Function | Implementation | Rationale |
|---|---|---|
| SHA-256 | Written from specification, validated against NIST test vectors | Implemented to understand it; hash functions are straightforward to verify exhaustively |
| ECDSA-P256 | [micro-ecc](https://github.com/kmackay/micro-ecc) | Signature verification has subtle side-channel and input-validation failure modes; a reviewed implementation is the correct choice |

## Repository layout

```
bootloader/     bootloader sources, SHA_256 implementation and linker script
app/            demo application, linked at 0x08020000
tools/          host-side image signing (Correct and corrupted) and Python script for signing
tests/          SHA-256 test vectors, host-side verification harness
docs/           threat model, design notes, engineering log
```

## Roadmap

- [x] Phase 0 — threat model, repo, concepts
- [x] Phase 1 — bootloader jumps to application
- [x] Phase 2 — SHA-256 integrity check, tampered image refused
- [ ] Phase 3 — ECDSA signature verification, wrong-key image refused
- [ ] Phase 4 — flash write protection, RDP, anti-rollback counter
- [ ] Phase 5 — signed firmware update over UART with A/B slots
- [ ] Phase 6 — demo video, writeup

## Building

*(to be filled in)*

## Security note

The signing private key is never committed to this repository. `keys/*.pem` is
gitignored. Public keys and test keys only.
