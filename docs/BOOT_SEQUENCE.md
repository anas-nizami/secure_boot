# Boot Sequence — power-on to application

## 1. Power-on reset

Reset circuitry releases the core once supply is stable.

## 2. Boot mode selection

Hardware samples BOOT0/BOOT1 at the reset edge to decide what is aliased
to address 0x00000000:

| BOOT1 | BOOT0 | Aliased to 0x00000000 | Runs                    |
|-------|-------|-----------------------|-------------------------|
| x     | 0     | Main flash 0x08000000 | This bootloader         |
| 0     | 1     | System memory ROM     | ST DFU/UART loader      |
| 1     | 1     | SRAM 0x20000000       | Whatever is in RAM      |

On the Discovery board BOOT0 is jumpered low, so main flash is selected.
Security note: an attacker able to pull BOOT0 high reaches ST's factory
loader and bypasses this bootloader entirely without touching its code.

## 3. Core fetches the first two vectors

The Cortex-M4 unconditionally reads:
  word 0 (offset 0x00) -> initial MSP
  word 1 (offset 0x04) -> initial PC (Reset_Handler)
The core has no knowledge of what is mapped there; BOOT0/BOOT1 decided that.

## 4. Bootloader Reset_Handler

Startup code copies .data from flash to RAM, zeroes .bss  section, and then
branches to main().

## 5. Bootloader main()

Initialises GPIO and clocks, then blinks the LEDs once as a proof-of-life
indicator — it confirms the bootloader started and reached main() before any
verification runs. The routine must RETURN; if it loops forever the chain
below is never reached.

Verification (steps 6-10) follows.

## 6. Read and validate the image header

The header occupies the 512 bytes at 0x08020000, immediately before the
application body.
It is passive data — no code, never executed — but it is
attacker-writable flash and is treated as untrusted input throughout.

Checks run cheapest first:

**Magic.** hdr->magic must equal 0x4E495A41 (AZIN). Erased flash reads 0xFFFFFFFF,
so this single comparison separates "no image present" from "image present".
Without it, img_len would read as 0xFFFFFFFF and the hash would run off the
end of flash.

**Length bounds.** img_len must be non-zero and fit within
APP_SLOT_SIZE - IMG_HEADER_SIZE. This is a security check, not a sanity
check: img_len is attacker-controlled.

Neither defends against a capable attacker, who would write a correct magic
and a plausible length. They defend against blank flash, partially written
flash, and a raw image flashed without a header.

## 7. Compute the image digest

SHA-256 over two regions, in order:

  1. the 16-byte header prefix — magic, version, img_len, reserved
  2. the application body — img_len bytes from 0x08020200

The digest covers the header fields as well as the body.
An earlier version hashed the body alone, which left version and img_len unauthenticated: the
version could be edited with a hex editor without breaking the signature,
defeating the rollback counter entirely.

The hash excludes the header's own hash, sig and pad fields — the first
cannot cover itself, and the second is derived from it.

Flash is memory-mapped, so both regions are read through plain pointers.

## 8. Integrity check

memcmp against hdr->hash. A mismatch means the image changed after signing.

memcmp rather than a constant-time comparison: timing leakage matters when
the compared value is a secret being guessed byte by byte. The stored digest
is not secret — it is derived from an image the attacker already has — and
preimage resistance means partial-match information cannot be used to
construct a matching image.

This step is a cheap early-out, not the load-bearing check. It fails in
microseconds where signature verification takes hundreds of milliseconds.

## 9. Signature verification

uECC_verify(g_pubkey, computed, 32, hdr->sig, uECC_secp256r1())

Verified against the locally computed digest, never against hdr->hash.
Verifying against the stored value would make this check depend on step 8
having run correctly; against the computed value it is valid on its own terms.

The public key is compiled into bootloader flash, generated from
keys/public-key.pem at build time. Verification requires no random number
generator — it is deterministic over public values. The device only verifies
and never signs, so no entropy source is needed on target.

This is the step that establishes authenticity. Step 8 establishes only that
the image is intact; an attacker who modifies the body can recompute its
digest, but cannot produce a valid signature over it.

## 10. Rollback check

stored = number of leading 0x00 bytes in sector 4 (0x08010000)

  version <  stored   refuse — downgrade attempt
  version == stored   boot, write nothing
  version >  stored   write (version - stored) bytes of 0x00, then boot

Reached only after signature verification, because version is untrusted
until then.

Signatures prove authorship, not freshness. Without this check a genuinely
signed but known-vulnerable release could be reflashed and would pass every
preceding check.

The counter is advanced before the handoff, since the bootloader does not run
again afterwards. Known limitation: an image that fails at runtime therefore
locks out the previous working version. The intended fix is a trial boot
where the application confirms health before the counter commits, with the
MPU preventing the application from writing sector 4 directly.

## 11. Refuse path

Any failed check calls refuse(), which drives the red LED and halts. There is
no path from a failed check to the handoff — the function does not return.
This is the property the whole design rests on.

## 12. Branch to the application's Reset_Handler

entry = *(uint32_t *)0x08020204  (word 1 of the app's vector table)
Note: this is the Reset_Handler, NOT main(). The address is odd — bit 0
set indicates Thumb state. Branching to an even address faults, as the
M4 has no ARM mode.

## 13. Application startup runs

The app performs its own .data copy and .bss zero, then branches to its
main(). Steps 4-5 repeat for the second image. The application is a
complete standalone program and is unaware it was launched by a
bootloader.
