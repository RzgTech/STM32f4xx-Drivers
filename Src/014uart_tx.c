/*
 * 014uart_tx.c
 *
 *  Created on: 14 Apr 2026
 *      Author: Vahid
 */

#include<string.h>
#include<stdio.h>
#include "stm32f407xx.h"


void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

USART_Handle_t usart2_handle;

//message
char msg[1024] = "UART TX Testing...\n\r";


void USART2_GPIOInits()
{
	GPIO_Handle_t USARTPins;
	USARTPins.pGPIOx = GPIOA;
	USARTPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	USARTPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	USARTPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	USARTPins.GPIO_PinConfig.GPIO_PinAltFunMode = 7; //based on Alt func table
	USARTPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//USART2 TX
	USARTPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
	GPIO_Init(&USARTPins);

	//USART RX
	USARTPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GPIO_Init(&USARTPins);

}

void USART2_Inits(void)
{
	usart2_handle.pUSARTx = USART2;
	usart2_handle.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
	usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
	usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
	usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
	usart2_handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
	usart2_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;

	USART_Init(&usart2_handle);

}

void GPIO_BtnInit()
{

	GPIO_Handle_t GpioBtn;
	//this is button gpio configuration
	GpioBtn.pGPIOx = GPIOA;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioBtn);
}


int main(void)
{

	GPIO_BtnInit();
	//USART GPIO pin inits
	USART2_GPIOInits();

	//USART peripheral configurations
	USART2_Inits();

	//enable the USART peripheral
	USART_PeripheralControl(USART2, ENABLE);

	while(1)
	{
		//wait until button is pressed
		while (! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		//wait button de-bouncing related issues 200ms of delay
		delay();

		//send data to the receiver
		USART_SendData(&usart2_handle, (uint8_t *)msg, strlen(msg));

	}


}
