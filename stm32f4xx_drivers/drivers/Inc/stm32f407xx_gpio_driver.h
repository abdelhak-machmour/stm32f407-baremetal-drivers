/*
 * stm32f407xx_gpio_driver.h
 *
 *  Created on: 8 Jul 2026
 *      Author: Abdelhak MACHMOUR
 */

#ifndef INC_STM32F407XX_GPIO_DRIVER_H_
#define INC_STM32F407XX_GPIO_DRIVER_H_

#include "stm32f407xx.h"


/*
 * GPIO pin configuration structure
 */
typedef struct
{
    uint8_t GPIO_PinNumber;        /*!< Possible values from @GPIO_PIN_NUMBERS */
    uint8_t GPIO_PinMode;          /*!< Possible values from @GPIO_PIN_MODES */
    uint8_t GPIO_PinSpeed;         /*!< Possible values from @GPIO_PIN_SPEED */
    uint8_t GPIO_PinPuPdControl;   /*!< Possible values from @GPIO_PIN_PuPdControl */
    uint8_t GPIO_PinOPType;        /*!< Possible values from @GPIO_Pin_OPType */
    uint8_t GPIO_PinAltFunMode;    /*!< Possible values from @GPIO_Pin_AltFunMode */
} GPIO_PinConfig_t;


/*
 * GPIO handle structure
 */
typedef struct
{
    GPIO_RegDef_t *pGPIOx;              /* Holds the base address of the GPIO port */
    GPIO_PinConfig_t GPIO_PinConfig;    /* Holds the GPIO pin configuration settings */

} GPIO_Handle_t;


/*
 * @GPIO_PIN_MODES
 * GPIO pin possible modes
 */
#define GPIO_MODE_IN        0
#define GPIO_MODE_OUT       1
#define GPIO_MODE_ALTFN     2
#define GPIO_MODE_ANALOG    3
#define GPIO_MODE_IT_FT     4   // Input falling edge
#define GPIO_MODE_IT_RT     5   // Input rising edge
#define GPIO_MODE_IT_RFT    6   // Input rising and falling edges


/*
 * @GPIO_PIN_SPEED
 * GPIO pin possible output speeds
 */
#define GPIO_SPEED_LOW      0
#define GPIO_SPEED_MEDIUM   1
#define GPIO_SPEED_FAST     2
#define GPIO_SPEED_HIGH     3


/*
 * @GPIO_PIN_PuPdControl
 * GPIO pin pull-up and pull-down configuration macros
 */
#define GPIO_NO_PUPD        0
#define GPIO_PIN_PU         1
#define GPIO_PIN_PD         2


/*
 * @GPIO_Pin_OPType
 * GPIO pin possible output types
 */
#define GPIO_OP_TYPE_PP     0
#define GPIO_OP_TYPE_OD     1


/*
 * @GPIO_Pin_AltFunMode
 */


/*
 * @GPIO_PIN_NUMBERS
 */
#define GPIO_PIN_NO_0       0
#define GPIO_PIN_NO_1       1
#define GPIO_PIN_NO_2       2
#define GPIO_PIN_NO_3       3
#define GPIO_PIN_NO_4       4
#define GPIO_PIN_NO_5       5
#define GPIO_PIN_NO_6       6
#define GPIO_PIN_NO_7       7
#define GPIO_PIN_NO_8       8
#define GPIO_PIN_NO_9       9
#define GPIO_PIN_NO_10      10
#define GPIO_PIN_NO_11      11
#define GPIO_PIN_NO_12      12
#define GPIO_PIN_NO_13      13
#define GPIO_PIN_NO_14      14
#define GPIO_PIN_NO_15      15

/**********************************************************************
 * APIs supported by this driver
 * For more information about the APIs, refer to their definitions.
 **********************************************************************/

/*
 * Peripheral Clock Setup
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi);

/*
 * Initialization and De-initialization
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx);

/*
 * Data Read and Write
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);

void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx,
                           uint8_t PinNumber,
                           uint8_t Value);

void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value);

void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber);

/*
 * IRQ Configuration and ISR Handling
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void GPIO_IRQHandling(uint8_t PinNumber);



#endif /* INC_STM32F407XX_GPIO_DRIVER_H_ */
