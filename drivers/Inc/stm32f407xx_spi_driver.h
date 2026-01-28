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
	SPI_RegDef_t *pSPIx;  		//This holds the base address of the SPIx(1,2,3)
	SPI_Config_t SPIConfig;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxState;
	uint8_t RxState;

}SPI_Handle_t;

/*
 * @SPI_DeviceMode
 */

#define SPI_DEVICE_MODE_MASTER							1
#define SPI_DEVICE_MODE_SLAVE							0  //default value


/*
 * @SPI_BusConfig
 */

#define SPI_BUS_CONFIG_FD                				1
#define SPI_BUS_CONFIG_HD                				2
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY    				3

/*
 * @SPI_SclkSpeed
 */
#define SPI_SCLK_SPEED_DIV2             				0
#define SPI_SCLK_SPEED_DIV4             				1
#define SPI_SCLK_SPEED_DIV8             				2
#define SPI_SCLK_SPEED_DIV16             				3
#define SPI_SCLK_SPEED_DIV32             				4
#define SPI_SCLK_SPEED_DIV64             				5
#define SPI_SCLK_SPEED_DIV128             				6
#define SPI_SCLK_SPEED_DIV256             				7

/*
 * @SPI_DFF
 */

#define SPI_DFF_8BITS									0  //this is the default value
#define SPI_DFF_16BITS									1

/*
 * @SPI_CPOL
 */

#define SPI_CPOL_HIGH									1
#define SPI_CPOL_LOW									0

/*
 * @SPI_CPHA
 */
#define SPI_CPHA_HIGH									1
#define SPI_CPHA_LOW									0

/*
 * @SPI_SSM
 */

#define SPI_SSM_EN										1
#define SPI_SSM_DI										0  //default value


/*
 * SPI related status flags definitions
 */

#define SPI_TXE_FLAG    								( 1 << SPI_SR_TXE)
#define SPI_RXNE_FLAG   								( 1 << SPI_SR_RXNE)
#define SPI_BUSY_FLAG   								( 1 << SPI_SR_BSY)

/*
 * SPI Application Status
 */

#define SPI_READY										0
#define SPI_BUSY_IN_RX									1
#define SPI_BUSY_IN_TX									2


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

void SPI_Init(SPI_Handle_t *pSPI_Handle_t);
void SPI_DeInit(SPI_RegDef_t *pSPIx);


/*
 * Data Send and Receive
 */

void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t len);  //length should be always 32 or higher
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t len);


/*
 * IRQ COnfiguration and ISR handling
 */

void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);

/*
 * Other Peripheral Control APIs
 */

void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);


#endif /* INC_STM32F407XX_SPI_DRIVER_H_ */



