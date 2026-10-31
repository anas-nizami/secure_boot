/*
 * uart.h
 *
 *  Created on: Sep 30, 2026
 *      Author: anasn
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

#define RCC_BASE 		0x40023800U
#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_BASE		0x40020000U
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x0u))
#define GPIOA_AFRL      (*(volatile uint32_t *)(GPIOA_BASE + 0x20u))

#define USART2_BASE		0x40004400U
#define USART2_BRR      (*(volatile uint32_t *)(USART2_BASE + 0x08u))
#define USART2_CR1      (*(volatile uint32_t *)(USART2_BASE + 0x0Cu))
#define USART2_SR 	    (*(volatile uint32_t *)(USART2_BASE + 0x00u))
#define USART2_DR 	    (*(volatile uint32_t *)(USART2_BASE + 0x04u))


void uart_init(void);
void uart_putc(char c);
int  uart_getc(void);      /* blocking */

#endif /* UART_H_ */
