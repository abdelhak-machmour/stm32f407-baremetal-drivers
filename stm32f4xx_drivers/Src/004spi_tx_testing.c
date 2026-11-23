/*
 * 004spi_tx_testing.c
 *
 *  Created on: 23 Sep 2026
 *      Author: Abdelhak MACHMOUR
 */

#include "stm32f407xx.h"
#include <string.h>

/*
 * PB12  -> SPI2_NSS
 * PB13 -> SPI2_SCLK
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 *
 * ALT function mode : 5
 */

void SPI2_GPIOInits(void)
{
    GPIO_Handle_t SPIPins;

    SPIPins.pGPIOx = GPIOB;
    SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
    SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    // SCLK
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&SPIPins);

    // MOSI
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);

    // MISO
    //SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    //GPIO_Init(&SPIPins);

    // NSS
    //SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    //GPIO_Init(&SPIPins);
}

void SPI2_Inits(void)
{
    SPI_Handle_t SPI2handle;

    // Select SPI2 peripheral
    SPI2handle.pSPIx = SPI2;

    // Configure SPI in full-duplex mode
    SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;

    // Configure SPI2 as master
    SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;

    // Set the SPI clock prescaler to 2
    // This generates an SCLK of 8 MHz
    SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV2;

    // Configure 8-bit data frame format
    SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;

    // Clock polarity: LOW
    SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;

    // Clock phase: LOW
    SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;

    // Enable software slave management for the NSS pin
    SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_EN;

    // Initialize the SPI2 peripheral
    SPI_Init(&SPI2handle);
}

int main(void)
{
    char user_data[] = "Hello World";

    // Initialize the GPIO pins to behave as SPI2 pins
    SPI2_GPIOInits();

    // Initialize the SPI2 peripheral parameters
    SPI2_Inits();

    // Makes NSS internally high and avoids MODF error
    SPI_SSIConfig(SPI2, ENABLE);

    // Enable the SPI peripheral
    SPI_PeripheralControl(SPI2, ENABLE);

    // Transmit "Hello World" over the SPI2 MOSI line
    SPI_SendData(SPI2, (uint8_t *)user_data, strlen(user_data));

    // Confirm SPI is not busy
    while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));

    // Disable SPI2 (NSS goes high)
    SPI_PeripheralControl(SPI2, DISABLE);

    // Keep the program running
    while (1);

    return 0;
}









