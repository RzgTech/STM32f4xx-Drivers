/*
 * stm32f407xx_spi_driver.h
 *
 *  Created on: Jan 17, 2026
 *      Author: Vahid
 */

#ifndef INC_STM32F407XX_SPI_DRIVER_H_
#define INC_STM32F407XX_SPI_DRIVER_H_

#include "stm32f407xx.h"


/*
 *  Configuration structure for SPIx peripheral
 */
typedef struct
{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
}SPI_Config_t;


//handle structure for SPIx peripheral

typedef struct
{
	//pointer to hold the base address of the GPIO peripheral
	GPIO_RegDef_t *pSPIx;  		//This holds the base address of the SPIx(1,2,3)
	SPI_Config_t SPIConfig;

}SPI_Handle_t;




/*****************************************************************************************
 * 						APIS supported by this driver
 * 			For more information about the APIs check the function definitions
 *****************************************************************************************/

/*
 * Peripheral clock setup
 */

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnorDi);

/*
 * Init and DeInit
 */

void SPI_Init(GPIO_Handle_t *pSPI_Handle_t);
void SPI_DeInit(GPIO_RegDef_t *pSPIx);


/*
 * Data Send and Receive
 */

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len);  //length should be always 32 or higher
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len);

/*
 * IRQ COnfiguration and ISR handling
 */

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);



#endif /* INC_STM32F407XX_SPI_DRIVER_H_ */



