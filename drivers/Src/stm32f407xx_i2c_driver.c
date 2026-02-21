/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: 17 Feb 2026
 *      Author: Vahid
 */

#include "stm32f407xx.h"
#include "stm32f407xx_i2c_driver.h"

uint16_t AHB_PreScaler[8] = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t APB1_PreScaler[4] = {2, 4, 8, 16};
static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);

/*********************************************************************
 * @fn      		  - I2C_PeripheralControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pI2Cx->CR1 |= (1 << I2C_CR1_PE);
		//pI2cBaseAddress->CR1 |= I2C_CR1_PE_Bit_Mask;
	}else
	{
		pI2Cx->CR1 &= ~(1 << 0);
	}

}

/*********************************************************************
 * @fn      		  - I2C_PeriClockControl
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if(pI2Cx == I2C1)
		{
			I2C1EN_PCLK_EN();
		} else if (pI2Cx == I2C2)
		{
			I2C1EN_PCLK_EN();
		} else if (pI2Cx == I2C3)
		{
			I2C1EN_PCLK_EN();
		}
	}
	else
	{
		if(pI2Cx == I2C1)
		{
			I2C1EN_PCLK_DI();
		} else if (pI2Cx == I2C2)
		{
			I2C1EN_PCLK_DI();
		} else if (pI2Cx == I2C3)
		{
			I2C1EN_PCLK_DI();
		}
	}
}

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

	//for apb
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

/*********************************************************************
 * @fn      		  - I2C_Init
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -

 */
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempreg = 0;


	//ack control bit
	tempreg |= (pI2CHandle->I2C_Config.I2C_ACKControl << 10);
	pI2CHandle->pI2Cx->CR1 = tempreg;   //the reset value of the register is 0 so we can use "="

	//configure the FREQ field of CR2
	tempreg = 0;
	tempreg |= RCC_GetPCLK1Value()/1000000U;
	pI2CHandle->pI2Cx->CR2 = (tempreg & 0x3F);   //the reset value of the register is 0 so we can use "="

	//program the device own address
	tempreg = 0;
	tempreg |= (pI2CHandle->I2C_Config.I2C_DeviceAddress << 1);
	tempreg |= (1 << 14); //based on ref. manual. we dont know the reason!!!
	pI2CHandle->pI2Cx->OAR1 = tempreg;

	//CCR calculations
	uint16_t ccr_value = 0;
	tempreg = 0;
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed == I2C_SCLSpeed_SM)
	{
		//mode is standard mode
		ccr_value = RCC_GetPCLK1Value()/(2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);   //ccr = f_pclk1/(2*f_scl)
		tempreg |= (ccr_value & 0xFFF);  //ccr value is 12 bits
		pI2CHandle->pI2Cx->CCR = tempreg;
	}else
	{
		//mode is fast mode
		tempreg |= (1<<15);
		tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
		if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
		{
			ccr_value = RCC_GetPCLK1Value()/(2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}else
		{
			ccr_value = RCC_GetPCLK1Value()/(25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
		}

	}

	tempreg |= (ccr_value & 0xFFF);

	pI2CHandle->pI2Cx->CCR = tempreg;

}


void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{

}

void I2C_MasterSendData(I2C_Handle_t *pI2C_Handle_t, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr)
{
	//1. Generate the START condition
	I2C_GenerateStartCondition(pI2C_Handle_t->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in the SR1
	//   Note: Until SB is cleared SCL will be stretched (pulled to LOW)
	while(I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_SB))
	{

	}

}

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)  //we use static bcs it is private to this driver
{
	pI2Cx->CR1 |= (1 << I2C_CR1_START);
}



