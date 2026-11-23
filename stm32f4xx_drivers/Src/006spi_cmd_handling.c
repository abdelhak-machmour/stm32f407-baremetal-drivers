/*
 * 006spi_cmd_handling.c
 *
 *  Created on: 7 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */


#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

extern void initialise_monitor_handles();

/*
 * PB12 -> SPI2_NSS
 * PB13 -> SPI2_SCLK
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 *
 * Alternate function mode: 5
 */
 
 
// Command codes (the codes the slave recognizes)
#define COMMAND_LED_CTRL      0x50
#define COMMAND_SENSOR_READ   0x51
#define COMMAND_LED_READ      0x52
#define COMMAND_PRINT         0x53
#define COMMAND_ID_READ       0x54

#define LED_ON    1
#define LED_OFF   0

// Arduino analog pins
#define ANALOG_PIN0   0
#define ANALOG_PIN1   1
#define ANALOG_PIN2   2
#define ANALOG_PIN3   3
#define ANALOG_PIN4   4

// Arduino LED
#define LED_PIN   9

void delay(void)
{
    for(uint32_t i = 0; i < 500000/2; i++);
}

void SPI2_GPIOInits(void)
{
    GPIO_Handle_t SPIPins;

    SPIPins.pGPIOx = GPIOB;
    SPIPins.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode  = 5;
    SPIPins.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_FAST;

    // SCLK
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&SPIPins);

    // MOSI
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);

    // MISO
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    GPIO_Init(&SPIPins);

    // NSS
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    GPIO_Init(&SPIPins);
}

void SPI2_Inits(void)
{
    SPI_Handle_t SPI2handle;

    SPI2handle.pSPIx = SPI2;
    SPI2handle.SPIConfig.SPI_BusConfig  = SPI_BUS_CONFIG_FD;        // Full duplex
    SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;   // Master
    SPI2handle.SPIConfig.SPI_SclkSpeed  = SPI_SCLK_SPEED_DIV8;      // 16 MHz / 8 = 2 MHz
    SPI2handle.SPIConfig.SPI_DFF        = SPI_DFF_8BITS;            // 8-bit frames
    SPI2handle.SPIConfig.SPI_CPOL       = SPI_CPOL_LOW;
    SPI2handle.SPIConfig.SPI_CPHA       = SPI_CPHA_LOW;
    SPI2handle.SPIConfig.SPI_SSM        = SPI_SSM_DI;               // Hardware slave management

    SPI_Init(&SPI2handle);
}

void GPIO_ButtonInit(void)
{
    GPIO_Handle_t GPIOBtn;

    /* Button configuration */
    GPIOBtn.pGPIOx = GPIOA;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GPIOBtn);
}

uint8_t SPI_VerifyResponse(uint8_t ackbyte)
{
    if (ackbyte == 0xF5)
    {
        // ACK
        return 1;
    }

    // NACK
    return 0;
}


int main(void)
{
    uint8_t dummy_write = 0xFF;
    uint8_t dummy_read;

    initialise_monitor_handles();

    printf("Application is running\n");

    GPIO_ButtonInit();

    // Configure the GPIO pins as SPI2 pins
    SPI2_GPIOInits();

    // Configure the SPI2 peripheral
    SPI2_Inits();

    printf("SPI Init. done\n");

    /*
     * SSOE = 1 enables the NSS output.
     * NSS is then managed by hardware: low when SPE = 1, high when SPE = 0.
     */
    SPI_SSOEConfig(SPI2, ENABLE);

    while (1)
    {
        // Wait until the button is pressed
        while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

        // Avoid button debouncing issues (about 200 ms)
        delay();

        // Enable SPI2 (NSS goes low)
        SPI_PeripheralControl(SPI2, ENABLE);

        // 1. CMD_LED_CTRL <pin no> <value>
        uint8_t commandCode = COMMAND_LED_CTRL;
        uint8_t ackbyte;
        uint8_t args[2];

        // Send the command
        SPI_SendData(SPI2, &commandCode, 1);

        // Do a dummy read to clear the RXNE flag
        SPI_ReceiveData(SPI2, &dummy_read, 1);

        // Send a dummy byte (1 byte) to fetch the response from the slave
        SPI_SendData(SPI2, &dummy_write, 1);

        // Read the ACK byte received
        SPI_ReceiveData(SPI2, &ackbyte, 1);

        if (SPI_VerifyResponse(ackbyte))
        {
            // Send the arguments
            args[0] = LED_PIN;
            args[1] = LED_ON;
            SPI_SendData(SPI2, args, 2);
            printf("CMD_LED_CTRL Executed\n");

        }
		
		// 2. CMD_SENSOR_READ <analog pin number>

		// Wait until the button is pressed
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		// Avoid button debouncing issues (about 200 ms)
		delay();

		commandCode = COMMAND_SENSOR_READ;

		// Send the command
		SPI_SendData(SPI2, &commandCode, 1);

		// Do a dummy read to clear the RXNE flag
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Send a dummy byte (1 byte) to fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		// Read the ACK byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		if (SPI_VerifyResponse(ackbyte))
		{
			// Send the argument (analog pin number)
			args[0] = ANALOG_PIN0;
			SPI_SendData(SPI2, args, 1);

			// Do a dummy read to clear the RXNE flag
			SPI_ReceiveData(SPI2, &dummy_read, 1);

			// Insert a delay so that the slave has time to prepare the data
			delay();

			// Send a dummy byte (1 byte) to fetch the response from the slave
			SPI_SendData(SPI2, &dummy_write, 1);

			uint8_t analog_read;
			SPI_ReceiveData(SPI2, &analog_read, 1);
            printf("CMD_SENSOR_READ %d\n", analog_read);
		}
		
		// 3. CMD_LED_READ <pin no>

		// Wait until the button is pressed
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		// Avoid button debouncing issues
		delay();

		commandCode = COMMAND_LED_READ;

		// Send the command
		SPI_SendData(SPI2, &commandCode, 1);

		// Dummy read to clear the RXNE flag
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Send a dummy byte to fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		// Read the ACK byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		if (SPI_VerifyResponse(ackbyte))
		{
			// Send the argument (pin number)
			args[0] = LED_PIN;
			SPI_SendData(SPI2, args, 1);

			// Dummy read to clear the RXNE flag
			SPI_ReceiveData(SPI2, &dummy_read, 1);

			// Give the slave time to prepare the data
			delay();

			// Send a dummy byte to fetch the LED status
			SPI_SendData(SPI2, &dummy_write, 1);

			uint8_t led_status;
			SPI_ReceiveData(SPI2, &led_status, 1);   // 1 = ON, 0 = OFF
            printf("CMD_LED_READ %d\n", led_status);

		}
		
		// 4. CMD_PRINT <len> <message>

		// Wait until the button is pressed
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		// Avoid button debouncing issues
		delay();

		commandCode = COMMAND_PRINT;

		// Send the command
		SPI_SendData(SPI2, &commandCode, 1);

		// Dummy read to clear the RXNE flag
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Send a dummy byte to fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		// Read the ACK byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		uint8_t message[] = "Hello ! How are you ??";

		if (SPI_VerifyResponse(ackbyte))
		{
			// Send the argument: the message length
			args[0] = strlen((char *)message);
			SPI_SendData(SPI2, args, 1);

			// Dummy read to clear the RXNE flag
			SPI_ReceiveData(SPI2, &dummy_read, 1);

			delay();

			// Send the message, one byte at a time
			for (uint32_t i = 0; i < args[0]; i++)
			{
				SPI_SendData(SPI2, &message[i], 1);
				SPI_ReceiveData(SPI2, &dummy_read, 1);   // clear RXNE after each byte
			}

            printf("CMD_PRINT Executed\n");
		}
		
		// 5. CMD_ID_READ

		// Wait until the button is pressed
		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));

		// Avoid button debouncing issues
		delay();

		commandCode = COMMAND_ID_READ;

		// Send the command
		SPI_SendData(SPI2, &commandCode, 1);

		// Dummy read to clear the RXNE flag
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Send a dummy byte to fetch the response from the slave
		SPI_SendData(SPI2, &dummy_write, 1);

		// Read the ACK byte received
		SPI_ReceiveData(SPI2, &ackbyte, 1);

		uint8_t id[11];

		if (SPI_VerifyResponse(ackbyte))
		{
			// Read the 10-byte board ID from the slave
			for (uint32_t i = 0; i < 10; i++)
			{
				// Send a dummy byte to fetch each byte from the slave
				SPI_SendData(SPI2, &dummy_write, 1);
				SPI_ReceiveData(SPI2, &id[i], 1);
			}

			id[10] = '\0';   // terminate the string
            printf("CMD_ID_READ : %s \n",id);

		}
		
        // Confirm SPI is not busy
        while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));

        // Disable SPI2 (NSS goes high)
        SPI_PeripheralControl(SPI2, DISABLE);

        printf("SPI communication Closed\n");

    }

    return 0;
}


