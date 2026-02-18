/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Jan 17, 2026
 *      Author: Vahid
 */

#include "stm32f407xx.h"
#include "stm32f407xx_spi_driver.h"


static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);

/*********************************************************************
 * @fn      		  - SPI_PeriClockControl
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

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_EN();
		} else if (pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		} else if (pSPIx == SPI3)
		{
			SPI3_PCLK_EN();
		}
	}
	else
	{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_DI();
		} else if (pSPIx == SPI2)
		{
			SPI2_PCLK_DI();
		} else if (pSPIx == SPI3)
		{
			SPI3_PCLK_DI();
		}
	}
}



/*********************************************************************
 * @fn      		  - SPI_Init
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


void SPI_Init(SPI_Handle_t *pSPIHandle)
{

	//enable the peripheral clock

	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

	uint32_t tempreg = 0;

	//1. configure the device mode

	tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR ;

	//2. Configure the bus config

	if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{
		//bidi mode should be cleared
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);

	}else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{
		//bidi mode should be set
		tempreg |= (1 << SPI_CR1_BIDIMODE);

	}else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		//bidi mode should be cleared
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);

		//RXONLY bit must be set
		tempreg |= (1 << SPI_CR1_RXONLY);

	}

	//3. configure the spi serial clock speed (baud rate)

	tempreg |= (pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR);

	//4. configure the DFF

	tempreg |= (pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF);

	//5. configure the CPOL

	tempreg |= (pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL);

	//6. configure the CPHA

	tempreg |= (pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA);

	//7. configure the SSM

	tempreg |= (pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM);


	pSPIHandle->pSPIx->CR1 = tempreg; //we can use assignment here (I did not understand why lecture 140)

}


/*********************************************************************
 * @fn      		  - SPI_DeInit
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
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
	if(pSPIx == SPI1)
	{
		SPI1_REG_RESET();
	} else if (pSPIx == SPI2)
	{
		SPI2_REG_RESET();
	} else if (pSPIx == SPI3)
	{
		SPI3_REG_RESET();
	}
}


uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if(pSPIx->SR & FlagName)
	{
		return FLAG_SET;
	}

	return FLAG_RESET;
}


/*********************************************************************
 * @fn      		  - SPI_SSIConfig
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
void  SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pSPIx->CR1 |=  (1 << SPI_CR1_SSI);
	}else
	{
		pSPIx->CR1 &=  ~(1 << SPI_CR1_SSI);
	}

}


/*********************************************************************
 * @fn      		  - SPI_SSOEConfig
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
void  SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pSPIx->CR2 |=  (1 << SPI_CR2_SSOE);
	}else
	{
		pSPIx->CR2 &=  ~(1 << SPI_CR2_SSOE);
	}

}

/*********************************************************************
 * @fn      		  - SPI_SendData
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - This is blocking call i.e. the function call waits until all the bytes are transmitted

 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
	while (Len > 0)
	{
		//1. wait until TXE is set
		while(SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

		//2. Check the DFF bit in CR1

		if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
		{
			//16 bit DFF
			//1. load the data into the DR
			pSPIx->DR = *((uint16_t *)pTxBuffer);
			Len--;
			Len--;
			(uint16_t *)pTxBuffer++;

		}else
		{
			//8 bit DFF
			//1. load the data into the DR
			pSPIx->DR = *pTxBuffer;
			Len--;
			pTxBuffer++;

		}

	}

}

/*********************************************************************
 * @fn      		  - SPI_ReceiveData
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
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
	while (Len > 0)
	{
		//1. wait until RXNE is set
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

		//2. Check the DFF bit in CR1

		if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
		{
			//16 bit DFF
			//1. load the data from DR to RX buffer address
			*((uint16_t *)pRxBuffer) = pSPIx->DR;
			Len--;
			Len--;
			(uint16_t *)pRxBuffer++;

		}else
		{
			//8 bit DFF
			//1. load the data into the DR
			*pRxBuffer = pSPIx->DR;
			Len--;
			pRxBuffer++;
		}
	}
}



/*********************************************************************
 * @fn      		  - SPI_PeripheralControl
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

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pSPIx->CR1 |= (1 << SPI_CR1_SPE);
	}else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SPE);
	}

}




/*********************************************************************
 * @fn      		  - SPI_IRQInterruptConfig
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

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			//ISER0 register
			*NVIC_ISER0 |= (1 << IRQNumber);

		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			//ISER1 register
			*NVIC_ISER1 |= (1 << (IRQNumber%32));

		}else if(IRQNumber >= 64 && IRQNumber < 96)
		{
			//ISER2 register
			*NVIC_ISER2 |= (1 << (IRQNumber%32));
		}
	}else
	{
		if(IRQNumber <= 31)
		{
			//ICER0 register
			*NVIC_ICER0 |= (1 << IRQNumber);

		}else if(IRQNumber > 31 && IRQNumber < 64)
		{
			//ICER1 register
			*NVIC_ICER1 |= (1 << (IRQNumber%32));

		}else if(IRQNumber >= 64 && IRQNumber < 96)
		{
			//ICER2 register
			*NVIC_ICER2 |= (1 << (IRQNumber%32));
		}
	}

}

/*********************************************************************
 * @fn      		  - SPI_IRQPriorityConfig
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


void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//1. first let's find the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);  // 8 - NO_PR_BITS_IMPLEMENTED = 4: 4 lower bits of each section in priority registers are not implemented.

	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);

}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t len)
{

	uint8_t state = pSPIHandle->TxState;

	if(state != SPI_BUSY_IN_TX)
	{

		//1 . Save the Tx buffer address and Len information in some global variables
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = len;
		//2.  Mark the SPI state as busy in transmission so that
		//    no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->TxState = SPI_BUSY_IN_TX;
		//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);

	}

	return state;
}



uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t len)
{
	uint8_t state = pSPIHandle->RxState;

	if(state != SPI_BUSY_IN_RX)
	{

		//1 . Save the Rx buffer address and Len information in some global variables
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = len;
		//2.  Mark the SPI state as busy in reception so that
		//    no other code can take over same SPI peripheral until reception is over
		pSPIHandle->RxState = SPI_BUSY_IN_RX;
		//3. Enable the RXNEIE control bit to get interrupt whenever RXNE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);

	}



	return state;

}



void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
	uint8_t temp1, temp2;

	//first lets check for TXE
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_TXE);

	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);

	if (temp1 && temp2)
	{
		//handle TXE
		spi_txe_interrupt_handle(pHandle);
	}

	//check for RXNE
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_RXNE);

	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);

	if (temp1 && temp2)
	{
		//handle RXNE
		spi_rxne_interrupt_handle(pHandle);
	}

	//check for ovr flag
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_OVR);

	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);

	if (temp1 && temp2)
	{
		//handle RXNE
		spi_ovr_err_interrupt_handle(pHandle);
	}


}

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
			//Check the DFF bit in CR1
			if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
			{
				//16 bit DFF
				//load the data into the DR
				pSPIHandle->pSPIx->DR = *((uint16_t *)pSPIHandle->pTxBuffer);
				pSPIHandle->TxLen--;
				pSPIHandle->TxLen--;
				(uint16_t *)pSPIHandle->pTxBuffer++;

			}else
			{
				//8 bit DFF
				//load the data into the DR
				pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
				pSPIHandle->TxLen--;
				pSPIHandle->pTxBuffer++;

			}

			if (!pSPIHandle->TxLen)
			{
				//TxLen is zero, so close the spi transmission and inform the application that
				//Tx is over.
				//this prevents interrupts from setting up of TXE flag
				SPI_CloseTransmission(pSPIHandle);
				SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);

			}

}



static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//Check the DFF bit in CR1
	if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
	{
		//16 bit DFF
		//load the data into the DR
		pSPIHandle->pSPIx->DR = *((uint16_t *)pSPIHandle->pRxBuffer);
		pSPIHandle->RxLen--;
		pSPIHandle->RxLen--;
		(uint16_t *)pSPIHandle->pRxBuffer++;

	}else
	{
		//8 bit DFF
		//load the data into the DR
		pSPIHandle->pSPIx->DR = *pSPIHandle->pRxBuffer;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;

	}

	if (!pSPIHandle->RxLen)
	{
		//TxLen is zero, so close the spi transmission and inform the application that
		//Tx is over.
		//this prevents interrupts from setting up of TXE flag
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);

	}

}


static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{

	//uint8_t temp;
	//1. clear the ovr flag
	if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		SPI_ClearOVRFlag(pSPIHandle);
	}

	//2. inform the application
	SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);




}


void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;

}


void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
}

void SPI_ClearOVRFlag(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	//1. clear the ovr flag
	temp = pSPIHandle->pSPIx->DR;
	temp = pSPIHandle->pSPIx->SR;


}



__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)  //this should be implemented by the application
{

	//This is a weak implementation . the user application may override this function.
}


