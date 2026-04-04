/*
 * stm32f407xx_usart_driver.c
 *
 *  Created on: 4 Apr 2026
 *      Author: Vahid
 */

#include "stm32f407xx.h"

/*
 * configuration structure for USARTx peripheral
 */
typedef struct
{
	uint8_t USART_Mode;
	uint32_t USART_Baud;
	uint8_t USART_NoOfStopBits;
	uint8_t USART_WordLength;
	uint8_t USART_ParityControl;
	uint8_t USART_HWFlowControl;
}USART_Config_t;

typedef struct
{
	USART_Config_t USART_Config;
	USART_RegDef_t *pUSARTx;
}USART_Handle_t;
