/*
 * stm32f407xx_rcc_driver.c
 *
 *  Created on: 6 Apr 2026
 *      Author: Vahid
 */

#include "stm32f407xx_rcc_driver.h"

uint16_t AHB_PreScaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t APB1_PreScaler[4] = {2, 4, 8, 16};
uint8_t APB2_PreScaler[4] = {2, 4, 8, 16};

uint32_t RCC_GetPLLOutputClock(void)
{
	return 0;
}

uint32_t RCC_GetPCLK1Value(void)
{
	uint32_t pclk1, SystemClk;
	uint8_t clksrc, temp, ahbp, apb1p;
	clksrc = ((RCC->CFGR) >> 2) & 0x3; // shifting and masking

	// for clock source
	if (clksrc == 0)
	{
		SystemClk = 16000000;
	}else if (clksrc == 1)
	{
		SystemClk = 8000000;
	}else if (clksrc == 2)
	{
		SystemClk = RCC_GetPLLOutputClock();  //we'll not implement it in this course
	}

	//for ahb
	temp = ((RCC->CFGR) >> 4) & 0xF;  //4 bits ->0xF

	if (temp < 8)
	{
		ahbp = 1;
	}else
	{
		ahbp = AHB_PreScaler[temp - 8];  //instead of writing different else if,
										//we used the AHB_PreScaler to use only one
	}									//else and decide about the prescaler value
										//with the value of temp - 8

	//for apb1
	temp = ((RCC->CFGR) >> 10) & 0x7;  //3 bits -> 0x7

	if (temp < 4)
	{
		apb1p = 1;
	}else
	{
		apb1p = APB1_PreScaler[temp - 4];
	}

	pclk1 = (SystemClk / ahbp) / apb1p;


	return pclk1;
}

uint32_t RCC_GetPCLK2Value(void)
{
	uint32_t pclk2, SystemClk;
	uint8_t clksrc, temp, ahbp, apb2p;
	clksrc = ((RCC->CFGR) >> 2) & 0x3; // shifting and masking

	// for clock source
	if (clksrc == 0)
	{
		SystemClk = 16000000;
	}else if (clksrc == 1)
	{
		SystemClk = 8000000;
	}else if (clksrc == 2)
	{
		SystemClk = RCC_GetPLLOutputClock();  //we'll not implement it in this course
	}

	//for ahb
	temp = ((RCC->CFGR) >> 4) & 0xF;  //4 bits ->0xF

	if (temp < 8)
	{
		ahbp = 1;
	}else
	{
		ahbp = AHB_PreScaler[temp - 8];  //instead of writing different else if,
										//we used the AHB_PreScaler to use only one
	}									//else and decide about the prescaler value
										//with the value of temp - 8

	//for apb2
	temp = ((RCC->CFGR) >> 13) & 0x7;  //3 bits -> 0x7

	if (temp < 4)
	{
		apb2p = 1;
	}else
	{
		apb2p = APB2_PreScaler[temp - 4];
	}

	pclk2 = (SystemClk / ahbp) / apb2p;


	return pclk2;

}
