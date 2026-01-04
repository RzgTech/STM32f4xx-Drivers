/*
 * main.c
 *
 *  Created on: Jan 2, 2026
 *      Author: Vahid
 */


#include "stm32f407xx.h"

int main(void)
{
	return 0;
}

void EXTI0_IRQHandler(void)
{
	//handle the interrupt

	GPIO_IRQHandling(0);
}
