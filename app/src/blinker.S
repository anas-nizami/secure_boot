/*============================================================================
 * STM32F407 Discovery - blink the green LED forever
 * GNU as (arm-none-eabi) syntax, for STM32CubeIDE / arm-none-eabi-gcc
 *
 *   PD12 = green
 *
 * Exposed to C as:
 *     extern void gpio_init(void);
 *     extern void blink_green(void);   // does not return
 *===========================================================================*/

    .syntax unified                 @ required: enables 32-bit Thumb-2 encodings
    .cpu    cortex-m4
    .fpu    softvfp
    .thumb

/*--------------------------------- constants -------------------------------*/
    .equ    RCC_BASE,       0x40023800
    .equ    RCC_AHB1ENR,    RCC_BASE + 0x30
    .equ    GPIOD_BASE,     0x40020C00
    .equ    GPIOD_MODER,    GPIOD_BASE + 0x00
    .equ    GPIOD_ODR,      GPIOD_BASE + 0x14

    .equ    GPIOD_EN,       (1 << 3)            @ RCC_AHB1ENR bit 3 = 0x08

    @ MODER is 2 bits per pin; 01b = general purpose output
    .equ    MODER_GREEN,    (1 << 24)           @ PD12
    .equ    MODER_ORANGE,   (1 << 26)           @ PD13
    .equ    MODER_RED,      (1 << 28)           @ PD14
    .equ    MODER_BLUE,     (1 << 30)           @ PD15
    .equ    MODER_MASK,     0xFF000000          @ both bits of pins 12-15
    .equ    MODER_OUTPUTS,  (MODER_GREEN | MODER_ORANGE | MODER_RED | MODER_BLUE)

    .equ    LED_GREEN,      (1 << 12)

    @ ~3 cycles per delay iteration. At the 16 MHz HSI default:
    @   16000000 / 3 ~= 5333333 iterations per second.
    .equ    HSEC,           2666666             @ half a second on / off

/*----------------------------------- code ----------------------------------*/
    .section .text

/*
 * gpio_init - enable the GPIOD clock, set PD12-PD15 to output mode.
 * Leaf function. Clobbers r0-r2 (caller-saved under AAPCS, nothing to save).
 */
    .global gpio_init
    .type   gpio_init, %function
    .thumb_func
gpio_init:
    @ --- enable the GPIOD peripheral clock (read-modify-write) --------------
    ldr     r0, =RCC_AHB1ENR
    ldr     r1, [r0]
    orr     r1, r1, #GPIOD_EN
    str     r1, [r0]
    ldr     r1, [r0]                @ read-back: let the clock settle before
                                    @ touching the GPIO block

    @ --- PD12-PD15 to general purpose output (read-modify-write) -----------
    ldr     r0, =GPIOD_MODER
    ldr     r1, [r0]
    ldr     r2, =MODER_MASK
    bic     r1, r1, r2              @ clear only pins 12-15
    ldr     r2, =MODER_OUTPUTS
    orr     r1, r1, r2              @ set them to 01b
    str     r1, [r0]

    bx      lr
    .size   gpio_init, . - gpio_init

/*
 * blink_green - toggle PD12 forever, half a second on, half a second off.
 * Does not return.
 *
 *   r1 = GPIOD_ODR address, held for the lifetime of the loop
 *   r0 = value written to ODR
 *   r3 = delay counter (consumed by delay)
 */
    .global blink_green
    .type   blink_green, %function
    .thumb_func
blink_green:
    push    {lr}                    @ non-leaf: the bl below clobbers lr
    ldr     r1, =GPIOD_ODR          @ loaded once; the loop re-enters below it

green_loop:
    movw    r0, #LED_GREEN          @ movw, not movs: 1<<12 exceeds the 8-bit
    str     r0, [r1]                @ immediate that movs can encode
    ldr     r3, =HSEC
    bl      delay

    movs    r0, #0                  @ 0 fits in movs
    str     r0, [r1]
    ldr     r3, =HSEC
    bl      delay

    b       green_loop              @ re-enter AFTER the ODR load

    pop     {pc}                    @ unreachable
    .size   blink_green, . - blink_green

/*
 * delay - busy-wait for r3 iterations. Leaf function, clobbers r3 only.
 */
    .type   delay, %function
    .thumb_func
delay:
    subs    r3, r3, #1              @ S-suffix: bne reads the flags this writes
    bne     delay
    bx      lr
    .size   delay, . - delay

    .ltorg                          @ emit the literal pool used by ldr =
    .end
