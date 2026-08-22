/*============================================================================
 * STM32F407 Discovery - GPIOD LED blink
 * GNU as (arm-none-eabi) syntax, for STM32CubeIDE / arm-none-eabi-gcc
 *
 *   PD12 = green, PD13 = orange, PD14 = red, PD15 = blue
 *
 * Exposed to C as:
 *     extern void gpio_init(void);
 *     extern void gpio_blink(void);
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

    .equ 	INIT_BLINK_CYCLE, 5				@During the init we will blink all 4 LED's 5 times.

    .equ    LED_GREEN,      (1 << 12)
    .equ    LED_ORANGE,     (1 << 13)
    .equ    LED_RED,        (1 << 14)
    .equ    LED_BLUE,       (1 << 15)

    .equ    QSEC,           1333333

/*----------------------------------- code ----------------------------------*/
    .section .text

/*
 * gpio_init - enable the GPIOD clock, set PD12-PD15 to output mode.
 * Clobbers r0-r2 (caller-saved under AAPCS, no save/restore needed).
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
 * gpio_blink - cycle the four LEDs forever. Does not return.
 * r1 holds the ODR address for the lifetime of the loop.
 */
    .global gpio_blink
    .type   gpio_blink, %function
    .thumb_func
gpio_blink:
    push    {r4, lr}                    @ r4 is callee-saved; lr is clobbered by bl
    ldr     r1, =GPIOD_ODR          	@ loaded here, NOT inherited from gpio_init
    mov		r4, #INIT_BLINK_CYCLE

blink_loop:
    bl		blink_green
    bl      delay

    bl		blink_orange
    bl      delay

    bl		blink_red
    bl      delay

    bl 		blink_blue
    bl      delay

    subs    r4, r4, #1              @ S-suffix: updates the flag for how many times the cycle has run
    bne     blink_loop              @ re-enter AFTER the ODR load

    str     r0, [r1]

    pop     {r4, pc}                @ restore r4 and return in one instruction
    .size   gpio_blink, . - gpio_blink

    .type   blink_red, %function
    .thumb_func
blink_red:
	movw    r0, #LED_RED
    str     r0, [r1]
    ldr     r3, =QSEC
    bx      lr
    .size   blink_red, . - blink_red

    .type   blink_orange, %function
    .thumb_func
blink_orange:
	movw    r0, #LED_ORANGE
    str     r0, [r1]
    ldr     r3, =QSEC
    bx      lr
    .size   blink_orange, . - blink_orange

	.type   blink_green, %function
    .thumb_func
blink_green:
	movw    r0, #LED_GREEN
    str     r0, [r1]
    ldr     r3, =QSEC
    bx      lr
    .size   blink_green, . - blink_green

.type   blink_blue, %function
    .thumb_func
blink_blue:
	movw    r0, #LED_BLUE
    str     r0, [r1]
    ldr     r3, =QSEC
    bx      lr
    .size   blink_blue, . - blink_blue

/*
 * set_red - drive PD14 high, other PORTD LEDs low. Callable from C.
 * Leaf function. Clobbers r0-r1.
 */
    .global set_red
    .type   set_red, %function
    .thumb_func
set_red:
    ldr     r1, =GPIOD_ODR          @ loaded here; r1 is NOT inherited
    movw    r0, #LED_RED
    str     r0, [r1]
    bx      lr
    .size   set_red, . - set_red

/*
 * delay - busy-wait for r3 iterations. Leaf function, clobbers r3 only.
 */
    .type   delay, %function
    .thumb_func
delay:
    subs    r3, r3, #1
    bne     delay
    bx      lr
    .size   delay, . - delay

    .ltorg                          @ emit the literal pool used by ldr =
    .end
