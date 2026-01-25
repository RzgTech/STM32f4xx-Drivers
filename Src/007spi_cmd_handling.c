/*
 * 007spi_cmd_handling.c
 *
 *  Created on: Jan 23, 2026
 *      Author: Vahid
 */


#include<string.h>
#include "stm32f407xx.h"


//command codes
#define COMMAND_LED_CTRL      		0x50
#define COMMAND_SENSOR_READ      	0x51
#define COMMAND_LED_READ      		0x52
#define COMMAND_PRINT      			0x53
#define COMMAND_ID_READ      		0x54

#define LED_ON     					1
#define LED_OFF    					0

//arduino analog pins
#define ANALOG_PIN0 				0
#define ANALOG_PIN1 				1
#define ANALOG_PIN2 				2
#define ANALOG_PIN3 				3
#define ANALOG_PIN4 				4

//arduino led

#define LED_PIN  					9

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

/*
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 * PB13 -> SPI2_SCLK
 * PB12 -> SPI2_NSS
 * ALT Function mode: 5
 */

void SPI2_GPIOInits()
{
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;

	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP; // SPI works with pp, and open drain is not needed
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	//MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(&SPIPins);

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);

}

void SPI2_Inits(void)
{
	SPI_Handle_t SPI2handle;
	SPI2handle.pSPIx = SPI2;
	SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV8; // max. possible speed : generates serial clock of 8 MHz
	SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_DI; //hardware slave management enabled for NSS pin - we dont need to use NSS for this exercise

	SPI_Init(&SPI2handle);



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

uint8_t SPI_VerifyResponse(ackbyte)
{
	if (ackbyte == 0xf5)
	{
		//ack
		return 1;
	}

	//nack
	return 0;

}

int main(void)
{
	uint8_t dummy_write = 0xff;
	uint8_t dummy_read;
	//this function is used to initialize the GPIO pins to behave as SPI2 pins
	SPI2_GPIOInits();

	GPIO_BtnInit();

	SPI2_Inits();

	/*
	* making SSOE 1 makes NSS output enable.
	* The NSS pin is automatically managed by the hardware.
	* i.e when SPE=1 , NSS will be pulled to low
	* and NSS pin will be high when SPE=0
	*/

	SPI_SSOEConfig(SPI2, ENABLE);

	while(1)
	{

		//wait until button is pressed
		while (! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		//to avoid button de-bouncing issues
		delay();

		//enable the SPI2 peripheral

		SPI_PeripheralControl(SPI2, ENABLE);

		//1. CMD_LED_CTRL  	<pin no(1)>     <value(1)>

		uint8_t commndcode = COMMAND_LED_CTRL;
		uint8_t ackbyte;
		uint8_t args[2];

		SPI_SendData(SPI2, &commndcode, 1); //slave receives this, if it supports it, it sends ACK, o.w, NACK

		//do dummy read to clear off the RXNE
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		//send some dummy bits (1 byte) fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		//read the ack byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		if (SPI_VerifyResponse(ackbyte))
		{
			//send arguments
			args[0] = LED_PIN;
			args[1] = LED_ON;

			SPI_SendData(SPI2, args, 2);
		}
		//End of COMMAND_LED_CTRL

		//2. CMD_SENOSR_READ   <analog pin number(1) >

		//wait until button is pressed
		while (! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		//to avoid button de-bouncing issues
		delay();

		commndcode = COMMAND_SENSOR_READ;

		SPI_SendData(SPI2, &commndcode, 1); //slave receives this, if it supports it, it sends ACK, o.w, NACK

		//do dummy read to clear off the RXNE
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		//send some dummy bits (1 byte) fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		//read the ack byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		if (SPI_VerifyResponse(ackbyte))
		{
			//send arguments
			args[0] = ANALOG_PIN0;

			SPI_SendData(SPI2, args, 1);

			//Do dummy read to clear off the RXNE
			SPI_ReceiveData(SPI2, &dummy_read, 1);

			//some delay to make slave ready with the data after the ADC conversion
			delay();

			//send some dummy bits (1 byte) fetch the response from the slave
			SPI_SendData(SPI2, &dummy_write, 1);

			uint8_t analog_read;
			SPI_ReceiveData(SPI2, &analog_read, 1);
		}

		//lets confirm SPI is not busy
		while( SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG));

		//disable the SPI Peripheral
		SPI_PeripheralControl(SPI2, DISABLE);
	}


	return 0;
}
