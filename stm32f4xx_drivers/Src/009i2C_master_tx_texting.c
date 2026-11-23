/*
 * 009i2C_master_tx_texting.c
 *
 *  Created on: 9 Nov 2026
 *  Author: Abdelhak MACHMOUR
 */


#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define MY_ADDR      0x68
#define SLAVE_ADDR   0x68    // address of the Arduino slave

uint8_t some_data[] = "We are testing I2C master Tx\n";

I2C_Handle_t I2C1Handle;

/*
 * PB6 -> SCL
 * PB9 -> SDA
 */

void delay(void)
{
    for (uint32_t i = 0; i < 500000 / 2; i++);
}

void I2C1_GPIOInits(void)
{
    GPIO_Handle_t I2CPins;

    I2CPins.pGPIOx = GPIOB;
    I2CPins.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode  = 4;
    I2CPins.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_OD;
    I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    I2CPins.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;

    // SCL
    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&I2CPins);

    // SDA
    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
    GPIO_Init(&I2CPins);
}

void I2C1_Inits(void)
{
    I2C1Handle.pI2Cx                         = I2C1;
    I2C1Handle.I2C_Config.I2C_ACKControl     = I2C_ACK_ENABLE;
    I2C1Handle.I2C_Config.I2C_DeviceAddress  = MY_ADDR;
    I2C1Handle.I2C_Config.I2C_FMDutyCycle    = I2C_FM_DUTY_2;
    I2C1Handle.I2C_Config.I2C_SCLSpeed       = I2C_SCL_SPEED_SM;

    I2C_Init(&I2C1Handle);
}

void GPIO_ButtonInit(void)
{
    GPIO_Handle_t GPIOBtn;

    /* Button configuration */
    GPIOBtn.pGPIOx = GPIOA;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_0;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GPIOBtn);
}

int main(void)
{
    // Button init
    GPIO_ButtonInit();

    // I2C pin inits
    I2C1_GPIOInits();

    // I2C peripheral configuration
    I2C1_Inits();

    // Enable the I2C peripheral
    I2C_PeripheralControl(I2C1, ENABLE);

    while (1)
    {
        // Wait until the button is pressed
        while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        // Avoid button debouncing issues (about 200 ms)
        delay();

        // Send data to the slave
        I2C_MasterSendData(&I2C1Handle, some_data, strlen((char *)some_data), SLAVE_ADDR, I2C_DISABLE_SR);
    }

    return 0;
}







