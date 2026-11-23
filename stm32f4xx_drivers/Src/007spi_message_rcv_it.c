/*
 * 007spi_message_rcv_it.c
 *
 *  Created on: 14 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */

/*
 * Note: follow these instructions to test this code
 * 1. Download this code to the STM32 board (master)
 * 2. Download the slave code (003SPISlaveUartReadOverSPI.ino) to the Arduino board (slave)
 * 3. Reset both boards
 * 4. Enable the SWV ITM data console to see the message
 * 5. Open the Arduino IDE serial monitor
 * 6. Type a message and send it (set the line ending to "carriage return")
 */


 /*
 * PB12 -> SPI2_NSS
 * PB13 -> SPI2_SCLK
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 *
 * Alternate function mode: 5
 */

#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define MAX_LEN 500

SPI_Handle_t SPI2handle;

char RcvBuff[MAX_LEN];

volatile char ReadByte;

volatile uint8_t rcvStop = 0;

/* Set in the interrupt handler of the Arduino interrupt GPIO (PD6) */
volatile uint8_t dataAvailable = 0;

void delay(void)
{
    for (uint32_t i = 0; i < 500000 / 2; i++);
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

/* Configures the GPIO pin on which the slave signals "data available" */
void Slave_GPIO_InterruptPinInit(void)
{
    GPIO_Handle_t spiIntPin;
    memset(&spiIntPin, 0, sizeof(spiIntPin));

    // PD6: falling-edge interrupt input
    spiIntPin.pGPIOx = GPIOD;
    spiIntPin.GPIO_PinConfig.GPIO_PinNumber      = GPIO_PIN_NO_6;
    spiIntPin.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_IT_FT;
    spiIntPin.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_SPEED_LOW;
    spiIntPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&spiIntPin);

    GPIO_IRQPriorityConfig(IRQ_NO_EXTI9_5, NVIC_IRQ_PRIO15);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);
}

int main(void)
{
    uint8_t dummy = 0xFF;

    printf("Application is running\n");

    Slave_GPIO_InterruptPinInit();

    // Configure the GPIO pins as SPI2 pins
    SPI2_GPIOInits();

    // Configure the SPI2 peripheral
    SPI2_Inits();

    printf("SPI init. done\n");

    /*
     * SSOE = 1 enables the NSS output.
     * NSS is then managed by hardware: low when SPE = 1, high when SPE = 0.
     */
    SPI_SSOEConfig(SPI2, ENABLE);

    // Enable the SPI2 interrupt in the NVIC
    SPI_IRQInterruptConfig(IRQ_NO_SPI2, ENABLE);

    while (1)
    {
        rcvStop = 0;

        // Wait for the data-available interrupt from the slave
        while (!dataAvailable);

        // Disable the PD6 interrupt while the message is being read
        GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, DISABLE);

        // Enable the SPI2 peripheral
        SPI_PeripheralControl(SPI2, ENABLE);

        while (!rcvStop)
        {
            // Fetch the data from the SPI peripheral byte by byte, in interrupt mode
            while (SPI_SendDataIT(&SPI2handle, &dummy, 1) == SPI_BUSY_IN_TX);
            while (SPI_ReceiveDataIT(&SPI2handle, &ReadByte, 1) == SPI_BUSY_IN_RX);
        }

        // Confirm SPI is not busy
        while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));

        // Disable SPI2 (NSS goes high)
        SPI_PeripheralControl(SPI2, DISABLE);

        printf("Rcvd data = %s\n", RcvBuff);

        dataAvailable = 0;

        // Re-enable the PD6 interrupt for the next message
        GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);
    }

    return 0;
}

/* SPI2 interrupt handler: called for the SPI2 events (TXE, RXNE, OVR) */
void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&SPI2handle);
}

/* Called by the driver on SPI events */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    static uint32_t i = 0;

    /*
     * On the Rx complete event, copy the received byte into the receive buffer.
     * '\0' marks the end of the message (rcvStop = 1).
     */
    if (AppEv == SPI_EVENT_RX_CMPLT)
    {
        RcvBuff[i++] = ReadByte;

        if (ReadByte == '\0' || i == MAX_LEN)
        {
            rcvStop = 1;
            RcvBuff[i - 1] = '\0';
            i = 0;
        }
    }
}

/* Slave data-available interrupt handler (PD6) */
void EXTI9_5_IRQHandler(void)
{
    GPIO_IRQHandling(GPIO_PIN_NO_6);
    dataAvailable = 1;
}
