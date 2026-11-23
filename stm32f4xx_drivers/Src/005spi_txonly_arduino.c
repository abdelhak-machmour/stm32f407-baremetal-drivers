/*
 * 005spi_txonly_arduino.c
 *
 *  Created on: 2 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */


#include "stm32f407xx.h"
#include <string.h>

/*
 * PB12 -> SPI2_NSS
 * PB13 -> SPI2_SCLK
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 *
 * Alternate function mode: 5
 */

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

    // MISO: not needed, the master doesn't receive anything
    // SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    // GPIO_Init(&SPIPins);

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


int main(void)
{
    char user_data[] = "Arduino est la marque d'une plateforme de prototypage open-source qui permet aux utilisateurs de créer des objets électroniques interactifs à partir de cartes électroniques matériellement libres sur lesquelles se trouve un microcontrôleur.";

    GPIO_ButtonInit();

    // Configure the GPIO pins as SPI2 pins
    SPI2_GPIOInits();

    // Configure the SPI2 peripheral
    SPI2_Inits();

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

		// First send the length information
		uint8_t dataLen = strlen(user_data);
		SPI_SendData(SPI2, &dataLen, 1);

		// Send the data over MOSI
		SPI_SendData(SPI2, (uint8_t *)user_data, strlen(user_data));

		// Confirm SPI is not busy
		while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));

		// Disable SPI2 (NSS goes high)
		SPI_PeripheralControl(SPI2, DISABLE);
    }

    return 0;
}

