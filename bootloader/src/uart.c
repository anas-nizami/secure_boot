/*
 * uart.c
 *
 *  Created on: Sep 30, 2026
 *      Author: anasn
 */

#include <uart.h>

/**
 * If using a Silicon Labs CP2102 Classic USB to UART Bridge
 * Connect
   STM32 PA2 (USART2_TX)  →  CP2102 RXD
   STM32 PA3 (USART2_RX)  ←  CP2102 TXD
 */

#define TIMEOUT 100
void uart_init(void)
{
	const uint8_t gpioa_bit = 0;
	RCC_AHB1ENR |= (1U << gpioa_bit);

	const uint8_t usart2_bit = 17;
	RCC_APB1ENR |= (1U << usart2_bit);

	const uint8_t gpioa_mode_bit = 0x2;
	GPIOA_MODER &= ~((3U << 4) | (3U << 6));  	//reset all bits
	GPIOA_MODER |= (gpioa_mode_bit << 4);  		//PA2 as alternate function
	GPIOA_MODER |= (gpioa_mode_bit << 6);  		//PA3 as alternate function

	const uint8_t gpioa_alternate_function_bit = 0x7;
	GPIOA_AFRL &= ~((0xFU << 8) | (0xFU << 12));		//reset all bits
	GPIOA_AFRL |= (gpioa_alternate_function_bit << 8);  //AF for PA2
	GPIOA_AFRL |= (gpioa_alternate_function_bit << 12); //AF for PA3

	/**
	 * Calculation of BAUD RATE:

	 * RCC_CFGR all zeros means reset defaults throughout:
	 * SWS = 00 → HSI selected, 16 MHz
	 * APB1 = 16 MHz, which is your USART2 clock.
	 *
	 * USARTDIV = f_PCLK / (16 × baud)
         = 16,000,000 / (16 × 115,200)
         = 8.680555555555556
     * Mantissa = 8 → BRR bits 15:4
	 * Fraction = 0.6805... × 16 = 10.89 → rounds to 11 → BRR bits 3:0
	 * So BRR = (8 << 4) | 11 = 0x8B.
	 */

	USART2_BRR = 0x8B;

	USART2_CR1 |= (1U << 3) | (1U << 2);  //TE and RE bits to enable transmitter and receiver
	USART2_CR1 |= (1U << 13);  //UE bit to enable USART2
}

void uart_putc(char c) 
{
    while (!(USART2_SR & (1 << 7))) { }   /* wait for TXE */
    USART2_DR = c;
}

int uart_getc(void) 
{
	uint32_t timeout_counter = TIMEOUT;

	if (USART2_SR & (1U << 3))
	{
	    volatile uint32_t dummy = USART2_SR;
		dummy = USART2_DR;
		(void)dummy;      	  /* clearing sequence */
	    return -2;            /* overrun */
	}

    while (!(USART2_SR & (1U << 5)))
	{
    	if (--timeout_counter == 0)
    		return -1;
    }   /* wait for RXNE */
    return (int)(USART2_DR & 0xFF);
}
