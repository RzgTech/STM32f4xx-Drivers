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
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2C_Handle_t);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2C_Handle_t);

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)  //we use static bcs it is private to this driver
{
	pI2Cx->CR1 |= (1 << I2C_CR1_START);
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = (SlaveAddr << 1);

	SlaveAddr &= ~(1);  //Slave address is slave address + r/nw bit = 0 (ADD0 must be zero for WRITE)

	pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = (SlaveAddr << 1);

	SlaveAddr |= 1;  //Slave address is slave address + r/nw bit = 1 (ADD0 must be 1 for READ)

	pI2Cx->DR = SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
	uint32_t dummy_read;
	//check for device mode
	if(pI2CHandle->pI2Cx->SR2 & ( 1 << I2C_SR2_MSL))
	{
		//device is in master mode
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize  == 1)
			{
				//first disable the ack
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

				//clear the ADDR flag ( read SR1 , read SR2)
				dummy_read = pI2CHandle->pI2Cx->SR1;
				dummy_read = pI2CHandle->pI2Cx->SR2;
				(void)dummy_read;
			}

		}
		else
		{
			//clear the ADDR flag ( read SR1 , read SR2)
			dummy_read = pI2CHandle->pI2Cx->SR1;
			dummy_read = pI2CHandle->pI2Cx->SR2;
			(void)dummy_read;

		}

	}
	else
	{
		//device is in slave mode
		//clear the ADDR flag ( read SR1 , read SR2)
		dummy_read = pI2CHandle->pI2Cx->SR1;
		dummy_read = pI2CHandle->pI2Cx->SR2;
		(void)dummy_read;
	}
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->CR1 |= (1 << I2C_CR1_STOP);
}


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

	//enable the clock for i2c peripheral
	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

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

	//T_rise configurations
	if (pI2CHandle->I2C_Config.I2C_SCLSpeed == I2C_SCLSpeed_SM)
	{
		//mode is standard mode
		tempreg = (RCC_GetPCLK1Value() / 1000000U) + 1;   //we add 1 according to the ref. manual -> (F_pclk1 * T_rise(max))+1

	}else
	{
		//mode is fast mode
		tempreg = ((RCC_GetPCLK1Value() * 300)/ 1000000U) + 1;

	}

	pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3F); //3F bcs we are masking 6 bits

}


void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{

}



uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if(pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}

	return FLAG_RESET;
}



void I2C_MasterSendData(I2C_Handle_t *pI2C_Handle_t, uint8_t *pTxbuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	//1. Generate the START condition
	I2C_GenerateStartCondition(pI2C_Handle_t->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in the SR1
	//   Note: Until SB is cleared SCL will be stretched (pulled to LOW)
	while(! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_SB));

	//3. Send the address of the slave with r/nw bit set to w(0) (total 8 bits ) -> since we are writing to the slave, R/w should be 0
	I2C_ExecuteAddressPhaseWrite(pI2C_Handle_t->pI2Cx, SlaveAddr);

	//4. Confirm that address phase is completed by checking the ADDR flag in the SR1
	while(! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_ADDR));

	//5. clear the ADDR flag according to its software sequence
	//   Note: Until ADDR is cleared SCL will be stretched (pulled to LOW)
	I2C_ClearADDRFlag(pI2C_Handle_t);

	//6. send the data until len becomes 0

	while (Len > 0)
	{
		while (! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_TXE)); //wait till TXE flag is set
		pI2C_Handle_t->pI2Cx->DR = *pTxbuffer;
		pTxbuffer++;
		Len--;
	}

	//7. when Len becomes zero wait for TXE=1 and BTF=1 before generating the STOP condition
	//   Note: TXE=1 , BTF=1 , means that both SR and DR are empty and next transmission should begin
	//   when BTF=1 SCL will be stretched (pulled to LOW)
	while (! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_TXE));
	while (! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_BTF));

	//8. Generate STOP condition and master need not to wait for the completion of stop condition.
	//   Note: generating STOP, automatically clears the BTF
	if(Sr == I2C_DISABLE_SR)
		I2C_GenerateStopCondition(pI2C_Handle_t->pI2Cx);
}

void I2C_MasterReceiveData(I2C_Handle_t *pI2C_Handle_t, uint8_t *pRxbuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	//1.Generate START condition
	I2C_GenerateStartCondition(pI2C_Handle_t->pI2Cx);

	//2.confirm that start generation was completed
	while(! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_SB));

	//3. send the address of the slave with r/nw bit set to R(1) (8bits) --> since we are reading from the slave, R/w should be 1
	I2C_ExecuteAddressPhaseRead(pI2C_Handle_t->pI2Cx, SlaveAddr);

	//4. wait until address phase is completed by checking the ADDR flag in the SR1
	while(! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, I2C_FLAG_ADDR));

	// Procedure to Read a single byte from Slave
	if(Len == 1)
	{
		//1. Disable Acking
		I2C_ManageAcking(pI2C_Handle_t->pI2Cx, I2C_ACK_DISABLE);

		//2. clear the ADDR flag according to its software sequence
		//   Note: Until ADDR is cleared SCL will be stretched (pulled to LOW)
		I2C_ClearADDRFlag(pI2C_Handle_t);

		//3. wait until RXNE = 1
		while (! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, SPI_FLAG_RXNE));

		//4. Generate STOP condition
		if(Sr == I2C_DISABLE_SR)
			I2C_GenerateStopCondition(pI2C_Handle_t->pI2Cx);

		//5. Read data in the buffer DR
		*pRxbuffer = pI2C_Handle_t->pI2Cx->DR;
	}

	//Procedure to read data from slave when len > 1
	if(Len > 1)
	{
		// clear ADDR flag
		I2C_ClearADDRFlag(pI2C_Handle_t);

		//read the data until Len is zero
		for(uint32_t i = Len; i > 0; i--)
		{
			//wait until RXNE = 1
			while (! I2C_GetFlagStatus(pI2C_Handle_t->pI2Cx, SPI_FLAG_RXNE));

			if(i == 2) // last 2 bytes remaining
			{
				//clear the ack bit
				I2C_ManageAcking(pI2C_Handle_t->pI2Cx, I2C_ACK_DISABLE);

				//generate STOP condition
				if(Sr == I2C_DISABLE_SR)
					I2C_GenerateStopCondition(pI2C_Handle_t->pI2Cx);
			}

			//read the data from data register DR into buffer
			*pRxbuffer = pI2C_Handle_t->pI2Cx->DR;
			// increment the buffer address
			pRxbuffer++;
		}
	}

	//re-enable ACKing
	if (pI2C_Handle_t->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2C_Handle_t->pI2Cx, I2C_ACK_ENABLE);
	}
}

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (IRQNumber <= 31)
		{
			*NVIC_ISER0 |= (1 << IRQNumber);

		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			*NVIC_ISER1 |= (1 << (IRQNumber%32));

		}else if (IRQNumber >= 64 && IRQNumber < 96)
		{
			*NVIC_ISER2 |= (1 << (IRQNumber%32));
		}
	}
	else
	{
		if (IRQNumber <= 31)
		{
			*NVIC_ICER0 |= (1 << IRQNumber);

		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			*NVIC_ICER1 |= (1 << (IRQNumber%32));

		}else if (IRQNumber >= 64 && IRQNumber < 96)
		{
			*NVIC_ICER2 |= (1 << (IRQNumber%32));
		}
	}
}

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//1. first let's find the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);  // 8 - NO_PR_BITS_IMPLEMENTED = 4: 4 lower bits of each section in priority registers are not implemented.

	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);

}

/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVTEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);
	}

	return busystate;
}

uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);
	}

	return busystate;

}


void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if(EnorDi == I2C_ACK_ENABLE)
	{
		pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
	}else
	{
		pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
	}
}


static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2C_Handle_t)
{
	if (pI2C_Handle_t->TxLen > 0)
	{
		//1. Load the data into DR
		pI2C_Handle_t->pI2Cx->DR = *(pI2C_Handle_t->pTxBuffer);

		//2. Decrement the Tx Len
		pI2C_Handle_t->TxLen--;

		//3. Increment the buffer address
		pI2C_Handle_t->pTxBuffer++;
	}
}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2C_Handle_t)
{
	//we have to do the data reception
	if (pI2C_Handle_t->RxSize == 1)
	{

		*pI2C_Handle_t->pRxBuffer = pI2C_Handle_t->pI2Cx->DR;
		pI2C_Handle_t->RxLen--;

	}

	if (pI2C_Handle_t->RxSize > 1)
	{
		if (pI2C_Handle_t->RxLen == 2)
		{
			//clear the ack bit
			I2C_ManageAcking(pI2C_Handle_t->pI2Cx, DISABLE);


		}

		//read DR
		*pI2C_Handle_t->pRxBuffer = pI2C_Handle_t->pI2Cx->DR;
		pI2C_Handle_t->pRxBuffer++;
		pI2C_Handle_t->RxLen--;

	}

	if (pI2C_Handle_t->RxLen == 0)
	{
		//close I2C data reception and notify the application
		//1. Generate the STop condition

		if (pI2C_Handle_t->Sr == I2C_DISABLE_SR)
			I2C_GenerateStopCondition(pI2C_Handle_t->pI2Cx);
		//2. close I2C rx
		I2C_CloseReceiveData(pI2C_Handle_t);

		//3. Notify the application
		I2C_ApplicationEventCallback(pI2C_Handle_t, I2C_EV_RX_CMPL);
	}
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;

	if(pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2CHandle->pI2Cx,ENABLE);
	}
}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->CR2 &= ~( 1 << I2C_CR2_ITEVTEN);


	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}


void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data)
{
	pI2C->DR = data;
}


uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C)
{
	return (uint8_t)pI2C->DR;
}


/*********************************************************************
 * @fn      		  - I2C_EV_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Interrupt handling for different I2C events (refer SR1)

 */
void I2C_EV_IRQHandling(I2C_Handle_t *pI2C_Handle_t)
{
	//Interrupt handling for both master and slave mode of a device

	uint32_t temp1, temp2, temp3;
	temp1 = (pI2C_Handle_t->pI2Cx->CR2) & (1 << I2C_CR2_ITEVTEN);
	temp2 = (pI2C_Handle_t->pI2Cx->CR2) & (1 << I2C_CR2_ITBUFEN);
	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_SB);

	//1. Handle For interrupt generated by SB event
	//	Note : SB flag is only applicable in Master mode
	if (temp1 && temp3)
	{
		//The interrupt is generated because of SB event
		//This block will not be executed in slave mode because for slave SB is always zero
		//In this block lets executed the address phase
		if (pI2C_Handle_t->TxRxState == I2C_BUSY_IN_TX)
		{
			I2C_ExecuteAddressPhaseWrite(pI2C_Handle_t->pI2Cx, pI2C_Handle_t->DevAddr);

		}else if(pI2C_Handle_t->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_ExecuteAddressPhaseRead(pI2C_Handle_t->pI2Cx, pI2C_Handle_t->DevAddr);
		}

	}

	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_ADDR);
	//2. Handle For interrupt generated by ADDR event
	//Note : When master mode : Address is sent
	//		 When Slave mode   : Address matched with own address
	if (temp1 && temp3)
	{
		//Interrupt is generated bcs of ADDR event
		I2C_ClearADDRFlag(pI2C_Handle_t);
	}

	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_BTF);
	//3. Handle For interrupt generated by BTF(Byte Transfer Finished) event
	if (temp1 && temp3)
	{
		//BTF flag is set
		if (pI2C_Handle_t->TxRxState == I2C_BUSY_IN_TX)
		{
			//make sure that TXE is also SET
			if (pI2C_Handle_t->pI2Cx->SR1 & (1 << I2C_SR1_TXE))
			{
				//BTF, TXE = 1 => an indication to close the transmission
				if (pI2C_Handle_t->TxLen == 0)
				{
					//1. Generate the STOP condition
					//if (pI2C_Handle_t->Sr == I2C_DISABLE_SR)
						//I2C_GenerateStopCondition(pI2C_Handle_t->pI2Cx);
					//2. Reset all the member elements of the handle structure
					I2C_CloseSendData(pI2C_Handle_t);

					//3. Notify the application about transmission complete
					I2C_ApplicationEventCallback(pI2C_Handle_t, I2C_EV_TX_CMPL);
				}

			}


		}else if(pI2C_Handle_t->TxRxState == I2C_BUSY_IN_RX)
		{
			//nothing to do!
		}
	}

	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_STOPF);
	//4. Handle For interrupt generated by STOPF event
	// Note : Stop detection flag is applicable only slave mode . For master this flag will never be set
	if (temp1 && temp3)
	{
		//STOPF flag is set
		//clear the STOPD (based on RM: 1) read SR1 2) write to CR1)
		//1) read SR1: is already done in temp 3 = ...
		pI2C_Handle_t->pI2Cx->CR2 |= 0x0000; //write to CR1 (we cannot write any value to it so we or with 0 to not chnage the values)

		//notify the application that STOPF is detected
		I2C_ApplicationEventCallback(pI2C_Handle_t, I2C_EV_STOP);
	}

	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_TXE);
	//5. Handle For interrupt generated by TXE event
	if (temp1 && temp2 && temp3)
	{
		if (pI2C_Handle_t->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
		{
			//TXE flag is set
			//we have to do the data transmission
			if (pI2C_Handle_t->TxRxState == I2C_BUSY_IN_TX)
			{
				I2C_MasterHandleTXEInterrupt(pI2C_Handle_t);
			}
		}else
		{
			//slave
			//make sure the slave is really in transmitter mode
			if (pI2C_Handle_t->pI2Cx->SR2 & (1 << I2C_SR2_TRA))
			{

				I2C_ApplicationEventCallback(pI2C_Handle_t, I2C_EV_DATA_REQ);
			}
		}

	}

	temp3 = (pI2C_Handle_t->pI2Cx->SR1) & (1 << I2C_SR1_RXNE);
	//6. Handle For interrupt generated by RXNE event
	if (temp1 && temp2 && temp3)
	{
		//Check device mode
		if (pI2C_Handle_t->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
		{
			//the device is master
			//RXNE flag is set
			if (pI2C_Handle_t->TxRxState == I2C_BUSY_IN_RX)
			{
				I2C_MasterHandleRXNEInterrupt(pI2C_Handle_t);
			}

		}else
		{
			//slave
			//make sure the slave is really in receiver mode
			if (pI2C_Handle_t->pI2Cx->SR2 & (1 << I2C_SR2_TRA))
			{

				I2C_ApplicationEventCallback(pI2C_Handle_t, I2C_EV_DATA_RCV);
			}

		}
	}

}



/*********************************************************************
 * @fn      		  - I2C_ER_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the code also define these macros in the driver
						header file
						#define I2C_ERROR_BERR  3
						#define I2C_ERROR_ARLO  4
						#define I2C_ERROR_AF    5
						#define I2C_ERROR_OVR   6
						#define I2C_ERROR_TIMEOUT 7

 */
void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	uint32_t temp1,temp2;

	//Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->CR2) & ( 1 << I2C_CR2_ITERREN);


	/***********************Check for Bus error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1<< I2C_SR1_BERR);
	if(temp1  && temp2 )
	{
		//This is Bus error

		//Implement the code to clear the buss error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_BERR);

		//Implement the code to notify the application about the error
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}

	/***********************Check for arbitration lost error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_ARLO);
	if(temp1  && temp2)
	{
		//This is arbitration lost error

		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_ARLO);

		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);

	}

	/***********************Check for ACK failure  error************************************/

	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_AF);
	if(temp1  && temp2)
	{
		//This is ACK failure error

		//Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_AF);

		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_AF);
	}

	/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_OVR);
	if(temp1  && temp2)
	{
		//This is Overrun/underrun

		//Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_OVR);

		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_OVR);
	}

	/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1  && temp2)
	{
		//This is Time out error

		//Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_TIMEOUT);

		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_TIMEOUT);
	}

}

