# Design Notes

Decisions taken during implementation, and the reasoning behind them. Recorded
here so the choices are reviewable rather than implicit in the code.

---

## Memory layout

### Bootloader flash capped at 64 KB

The linker script limits the bootloader to sectors 0–3 rather than the full
1 MB. Growth past that boundary becomes a link error instead of silently
overwriting the metadata sector. The cap is enforced now, before SHA-256 and
micro-ecc are linked in, so any future overrun is caught at build time.

### Bootloader heap set to zero

No dynamic allocation in the bootloader. Fragmentation and non-deterministic
allocation latency are unacceptable in a component with no recovery path, and
an allocation failure at boot has nowhere to go. All buffers are statically
sized. Setting the heap to zero also turns any accidental dependency on
`malloc` into a link error.

The application's linker script is independent and unaffected — the bootloader
constrains only itself.

### Application based at 0x08020200, not 0x08020000

The 512-byte offset accommodates the image header while keeping the
application's vector table 512-byte aligned. VTOR bits [8:0] are hardwired to
zero on the Cortex-M4, and the F407 has 98 vectors (392 bytes), which rounds up
to a 512-byte alignment requirement. A 128-byte header would place the vector
table at 0x08020080 and VTOR would silently refuse it — interrupts would then
dispatch through the bootloader's table with no fault to indicate why.

The alignment requirement is asserted in the application's linker script rather
than relied upon:

```
ASSERT(ORIGIN(FLASH) % 512 == 0, "App base must be 512-byte aligned for VTOR")
```

---

## Bootloader handoff

The transfer of control does four things in a fixed order, all of them
necessary:

1. **Mask interrupts** (`cpsid i`) — no interrupt may fire while the vector
   table and stack pointer are inconsistent with each other.
2. **De-initialise SysTick** — a timer interrupt during handoff would vector
   through a partially updated table.
3. **Relocate VTOR**, then **adopt the application's MSP** from word 0 of its
   vector table. The bootloader's stack is abandoned at this point; the two
   images share no RAM.
4. **`dsb` / `isb` before the branch** — the write to VTOR must land, and the
   pipeline must be flushed, before execution transfers. Without the barriers
   the branch can execute against a stale vector table. The failure is
   timing-dependent and does not reproduce reliably, which is precisely why the
   barriers are unconditional rather than added in response to a bug.

The branch target is word 1 of the application's vector table — its
`Reset_Handler`, not `main`. The application then runs its own startup: `.data`
copy, `.bss` zero, then `main`. It is a complete standalone program and is
unaware it was launched by a bootloader.

A note on omitting VTOR specifically: the handoff still succeeds without it,
and a polled application appears to run correctly. Only interrupt dispatch
breaks, and only once an interrupt is actually used. An LED blink test does not
exercise this, so the defect would surface much later and in an unrelated
subsystem.

---

## SHA-256

### Implemented from specification rather than imported

Hash functions can be validated exhaustively against published test vectors,
which makes a from-scratch implementation verifiable rather than merely
plausible. ECDSA is a different case — signature verification has
side-channel and input-validation failure modes that testing does not surface —
and a reviewed library (micro-ecc) is used there instead.

### Validating derived constants by regeneration

The round constants K[0..63] are the first 32 bits of the fractional parts of
the cube roots of the first 64 primes; the initial hash values H[0..7] use
square roots of the first 8 primes. Because they are *derived* rather than
arbitrary, they can be recomputed independently and diffed against the
transcribed table.

This matters because a single wrong hex digit produces a digest that is simply
incorrect, with nothing to indicate where the fault lies. Visual proofreading
against the same source page is unreliable — the eye repeats its own error.
Regeneration compares against an independent computation.

This is how the transposed digit in K[53] was located — the NIST vectors
reported a mismatched digest, but not which of 64 constants or four functions
was at fault. The technique generalises to any constant table with a generating
rule: CRC tables, trigonometric lookups, calibration curves.

### Padding buffer sized to the worst case, not runtime

`sha256_final` uses a fixed 72-byte padding buffer rather than a
variable-length array. 72 is the maximum, occurring at `buflen == 56`:
`(120 - 56) + 8`. The bound holds because `sha256_update` drains the buffer at
64 bytes, so `buflen` on entry to `final` is always 0–63.

VLAs are banned by MISRA and were removed from the Linux kernel for the same
reason: stack consumption depends on runtime data, and there is no failure path
to check. Where a worst case can be stated at design time, it should be.

### Streaming interface

`init` / `update` / `final` rather than a single call, because the bootloader
hashes flash in chunks. Stack usage stays flat regardless of image size, and
the hash state lives in a caller-provided context rather than in static storage.

---

## Verification ordering

Checks run cheapest-first: magic number, then image length bounds, then the
SHA-256 comparison, and (from Phase 3) signature verification. Malformed input
is rejected before any expensive computation.

The length field deserves particular attention. It is attacker-controlled data
read from flash, and an unbounded value would cause the bootloader to hash past
the end of the slot. It is validated against the slot size before use.

Every failure path terminates. There is no route from a failed check to the
jump — the refuse path signals and halts rather than returning to a caller that
might proceed.

---

## Testing

The host test suite compiles with `-fsanitize=undefined,address`. This is not
optional tooling: it identified a signed-overflow defect in the message
schedule endian conversion that produced correct output on this compiler and
would not have been found by inspection. `block[i]` is `uint8_t` and promotes
to `int`; `0x80 << 24` exceeds `INT_MAX`. The 0x80 padding byte guarantees the
condition occurs on every hash computed.

Validation covers the NIST CAVS ShortMsg vectors plus the empty string, "abc",
the 55- and 56-byte padding boundary cases, and a 1,000,000-byte message fed
through 1,000 separate `update` calls to exercise buffering across block
boundaries.

One observation worth recording: an earlier version passed `sizeof(padding)`
rather than `padding_length + 8` to `update`, and produced correct digests
regardless. The surplus zero bytes always land beyond the final transform
boundary and are discarded. Code that is accidentally correct is worse than
code that is wrong — the defect is invisible to testing and a later refactor
breaks it silently.

---

## Tooling

AI assistance was used for debugging and code review during development. All
bootloader and cryptographic implementation code is my own. `tests/parser.c` is
AI-generated test-harness support code and is marked as such in that file; no
parser code is compiled into the target image.
