/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: Jan 17, 2026
 *      Author: Vahid
 */


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
	GPIO_RegDef_t *pSPIx;  			//This holds the base address of the GPIO port to which the pin belongs
	SPI_Config_t SPIConfig;    //This holds GPIO pin configuration settings

}SPI_Handle_t;
