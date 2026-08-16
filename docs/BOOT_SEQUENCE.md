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

Startup assembly copies .data from flash to RAM, zeroes .bss, then
branches to main().

## 5. Bootloader main()

Initialises GPIO and runs the LED indication loop. This routine must
RETURN — if it loops forever the handoff below is never reached.

## 6. Mask interrupts

cpsid i — no interrupt may fire while the vector table and stack pointer
are in an inconsistent state.

## 7. De-initialise peripherals

SysTick CTRL/LOAD/VAL cleared. A timer interrupt during handoff would
vector through a partially updated table.

## 8. Relocate the vector table

SCB->VTOR = 0x08020200. Base must be 512-byte aligned: the F407 has 98
vectors (392 bytes), rounded up to the next power of two, and VTOR bits
[8:0] are hardwired to zero.

## 9. Adopt the application's stack pointer

msp = *(uint32_t *)0x08020200   (word 0 of the app's vector table)
msr msp, msp
The bootloader's stack is abandoned here. The two images share no RAM.

## 10. Memory barriers

dsb — ensure the VTOR write has landed
isb — flush the pipeline so prefetched instructions are discarded
Without these the branch can execute before VTOR takes effect. The
failure is timing-dependent and does not reproduce reliably.

## 11. Branch to the application's Reset_Handler

entry = *(uint32_t *)0x08020204   (word 1 of the app's vector table)
Note: this is the Reset_Handler, NOT main(). The address is odd — bit 0
set indicates Thumb state. Branching to an even address faults, as the
M4 has no ARM mode.

## 12. Application startup runs

The app performs its own .data copy and .bss zero, then branches to its
main(). Steps 4-5 repeat for the second image. The application is a
complete standalone program and is unaware it was launched by a
bootloader.