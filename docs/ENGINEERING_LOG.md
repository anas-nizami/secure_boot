# Engineering Log

---

## 2026-08-16 — Phase 1: bootloader handoff

**Goal:** Two independently linked images; bootloader transfers control
to an application at 0x08020200.

**What happened:** Created a second CubeIDE project with FLASH ORIGIN at
0x08020200, verified .isr_vector placement in the map file, added an
ASSERT to the linker script enforcing 512-byte alignment, and wrote
jump_to_app() in the bootloader. Blue LED (bootloader) followed by green
LED (app) on cold boot.

**Problems hit:**

1. App LED lit briefly then died when run standalone.
2. Bootloader reached solid blue and stopped — app never started.
3. No CMSIS in the project, so SCB / __set_MSP /__DSB were unavailable.

**How I diagnosed it:**

1. LED behaviour: momentary rather than sustained, pointing at control
   flow rather than GPIO configuration.
2. Solid rather than blinking blue meant the blink loop had completed and
   execution continued past it — so the fault was after the blink.
3. Build error on undefined symbols.

**Fixes:**

1. main() must never return on bare metal — there is nowhere to return
   to. Replaced `return 0` with an infinite loop.
2. jump_to_app() had not been written. Flash contents and execution are
   separate concerns; nothing runs by virtue of being in flash.
3. Replaced CMSIS calls with direct register access:
   SCB->VTOR   -> *(volatile uint32_t*)0xE000ED08
   __set_MSP   -> asm("msr msp, %0")
   __disable_irq -> asm("cpsid i")
   __DSB/__ISB -> asm("dsb"), asm("isb")

**Time lost:** ~1.5 hours, mostly on (2).

**Would do differently:** Test the app standalone before attempting the
handoff — it isolates "app is broken" from "jump is broken" in two
minutes and would have skipped most of the debugging.

**Carried forward:** Bootloader heap set to 0 (no dynamic allocation in a
bootloader — fragmentation and non-deterministic timing are unacceptable
where there is no recovery path). Bootloader FLASH length capped at 64K
so growth past sectors 0-3 becomes a link error
---
