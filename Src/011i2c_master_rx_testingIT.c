/*
 * 009i2c_master_tx_testing.c
 *
 *  Created on: 8 Mar 2026
 *      Author: Vahid
 */

#include<string.h>
#include<stdio.h>
#include "stm32f407xx.h"

#define MY_ADDR        0x61
#define SLAVE_ADDR	   0x68  //to be checked

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000 ; i ++);
}

I2C_Handle_t I2C1Handle;

//rcv buffer
uint8_t rcv_buf[32];

/*
 * PB6-> SCL
 * PB9-> SDA
 */

void I2C1_GPIOInits()
{
	GPIO_Handle_t I2CPins;
	I2CPins.pGPIOx = GPIOB;
	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4; //based on Alt func table
	I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//SCL
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&I2CPins);

	//SDA
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
	GPIO_Init(&I2CPins);

}

void I2C1_Inits(void)
{
	I2C1Handle.pI2Cx = I2C1;
	I2C1Handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;  //sth random -> you cannot use reserved addresses (refer to sepc. UM10204)
														//here we are master and the address does not matter. we usuallu assign address to the slave

	I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
	I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCLSpeed_SM;

	I2C_Init(&I2C1Handle);

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
	uint8_t command_code;
	uint8_t len;

	GPIO_BtnInit();
	//I2C pin inits
	I2C1_GPIOInits();

	//I2C peripheral configurations
	I2C1_Inits();

	//I2C IRQ configurations
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

	//enable the i2c peripheral
	I2C_PeripheralControl(I2C1, ENABLE);

	//ack bit is made 1 after PE = 1
	I2C_ManageAcking(I2C1, ENABLE);

	while(1)
	{
		//wait until button is pressed
		while (! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		//wait button de-bouncing related issues 200ms of delay
		delay();

		command_code = 0x51;

		while(I2C_MasterSendDataIT(&I2C1Handle, &command_code, 1, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);  //to keep it waiting in case it is in tx or rx
		while(I2C_MasterReceiveDataIT(&I2C1Handle, &len, 1, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);

		command_code = 0x52;
		while(I2C_MasterSendDataIT(&I2C1Handle, &command_code, 1, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);
		while(I2C_MasterReceiveDataIT(&I2C1Handle, rcv_buf, len, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);



	}


}


void I2C1_EV_IRQHandler(void)
{
	I2C_EV_IRQHandling(&I2C1Handle);
}

void I2C1_ER_IRQHandler(void)
{
	I2C_ER_IRQHandling(&I2C1Handle);
}


void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
	if (AppEv == I2C_EV_TX_CMPL)
	{
		printf("TX is completed\n");

	}else if (AppEv == I2C_EV_RX_CMPL)
	{
		printf("RX is completed\n");

	}else if (AppEv == I2C_ERROR_AF)
	{
		printf("Err: Ack failure\n");
		//In master ack failure happens when slave fails to send ack for the byte
		//sent from the master. -> what to conclude? Maybe the slave is removed from the bus or sth has happened to the slave or it does not want more data
		I2C_CloseSendData(pI2CHandle);
		I2C_GenerateStopCondition(I2C1);

		//hang in infinite loop
		while(1);


	}
}
