/*
 * 008spi_eeprom_interface_app.c
 *
 *  Created on: 17 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */
#include "stm32f407xx.h"
#include <string.h>
#include <stdio.h>

#define EEPROM_CS_LOW()   GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, GPIO_PIN_RESET)
#define EEPROM_CS_HIGH()  GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, GPIO_PIN_SET)

/*
 * PB12 -> SPI2_NSS
 * PB13 -> SPI2_SCLK
 * PB14 -> SPI2_MISO
 * PB15 -> SPI2_MOSI
 *
 * Alternate function mode: 5
 */

/*
 * STM32 pin		  25LC256 pin
 *
 * 	  PB12    ---->		CS
 *    PB14    ---->		SO
 *    GND     ---->		VSS
 *    PB15    ---->		SI
 *    PB13    ---->		SCK
 */

/*
 * Sequence steps :
 * 1. Initialize SPI2 and configure the GPIO pins.
 * 2. Configure PD2 (green LED), PD3 (red LED), and PD4 (button).
 * 3. Implement EEPROM_WriteEnable() using the 0x06 command.
 * 4. Implement EEPROM_Write() using interrupts.
 * 5. Wait for the EEPROM's internal write cycle to finish by polling the WIP bit.
 * 6. Implement EEPROM_WriteDisable() using 0x04.
 * 7. Implement EEPROM_Read() using interrupts.
 * 8. Verify the received data against the transmitted data.
 */

/*
 * 0x0000		NAME
 * 				32 octets | Name
 *
 * 0x0020		ID
 * 				4 octets | ID
 *
 * 0x0024       MODEL
 * 				8 octets | Aircraft Model
 *
 * 0x002C		DEPARTURE
 * 				16 octets | Departure
 *
 * 0x003C		DESTINATION
 * 				16 octets | Destination
 *
 * 0x004C		ALTITUDE
 * 				4 octets | Altitude
 *
 * 0x0050		SPEED
 * 				4 octets | Speed
 *
 */

#define NAME_ADDR         0x0000U
#define ID_ADDR           0x0020U
#define MODEL_ADDR        0x0024U
#define DEPARTURE_ADDR    0x002CU
#define DESTINATION_ADDR  0x003CU
#define ALTITUDE_ADDR     0x004CU
#define SPEED_ADDR        0x0050U

#define NAME_SIZE         32U
#define ID_SIZE            4U
#define MODEL_SIZE         8U
#define DEPARTURE_SIZE    16U
#define DESTINATION_SIZE  16U
#define ALTITUDE_SIZE      4U
#define SPEED_SIZE         4U

SPI_Handle_t SPI2handle;


typedef struct
{
    char name[32];
    uint32_t id;
    char aircraftModel[8];
    char departure[16];
    char destination[16];
    uint32_t altitude;
    uint32_t speed;
} FlightData;

void delay(void)
{
    for (uint32_t i = 0; i < 500000 / 2; i++);
}

// 1. Initialize SPI2 and configure the GPIO pins.

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
    SPI2handle.SPIConfig.SPI_SclkSpeed  = SPI_SCLK_SPEED_DIV4;      // 16 MHz / 4 = 4 MHz
    SPI2handle.SPIConfig.SPI_DFF        = SPI_DFF_8BITS;            // 8-bit frames
    SPI2handle.SPIConfig.SPI_CPOL       = SPI_CPOL_LOW;
    SPI2handle.SPIConfig.SPI_CPHA       = SPI_CPHA_LOW;
    SPI2handle.SPIConfig.SPI_SSM        = SPI_SSM_DI;               // Hardware slave management

    SPI_Init(&SPI2handle);
}

// 2. Configure PD2 (green LED), PB3 (red LED), and PD4 (button).

void LEDsInit(void)
{
	GPIO_Handle_t GpioLed1, GpioLed2;

	GpioLed1.pGPIOx = GPIOD;
	GpioLed1.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
	GpioLed1.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed1.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed1.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioLed1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioLed1);

	GpioLed2.pGPIOx = GPIOD;
	GpioLed2.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GpioLed2.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed2.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed2.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioLed2.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GpioLed2);
}

void GPIO_ButtonInit(void)
{
    GPIO_Handle_t GPIOBtn;

    /* Button configuration */
    GPIOBtn.pGPIOx = GPIOD;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&GPIOBtn);

    /* IRQ configuration */
    GPIO_IRQPriorityConfig(IRQ_NO_EXTI4, NVIC_IRQ_PRIO15);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI4, ENABLE);
}

static void EEPROM_SendCommand(uint8_t cmd)
{
    uint8_t received;

    EEPROM_CS_LOW();
    SPI_SendData(SPI2, &cmd, 1);
    SPI_ReceiveData(SPI2, &received, 1);
    while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));  // Confirm SPI is not busy
    EEPROM_CS_HIGH();
}

// Write Enable: set the Write Enable Latch (WEL)
void EEPROM_WriteEnable(void)  { EEPROM_SendCommand(0x06); }


// Write Disable: reset the Write Enable Latch (WEL)
void EEPROM_WriteDisable(void) { EEPROM_SendCommand(0x04); }

int main(void)
{
	uint8_t dummy = 0xFF;
	printf("Application is running\n");

	// 1. Initialize SPI2 and configure the GPIO pins.
	// Configure the GPIO pins as SPI2 pins
	SPI2_GPIOInits();

	// Configure the SPI2 peripheral
	SPI2_Inits();

	// 2. Configure PD2 (green LED), PB3 (red LED), and PD4 (button).
	LEDsInit();
	GPIO_ButtonInit();



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

	 }




}
