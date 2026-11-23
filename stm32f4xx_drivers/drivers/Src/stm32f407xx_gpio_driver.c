/*
 * stm32f407xx_gpio_driver.c
 *
 *  Created on: 8 Jul 2026
 *      Author: Abdelhak MACHMOUR
 */


#include "stm32f407xx_gpio_driver.h"

/************************************************************************************
 * @fn         - GPIO_PeriClockControl
 *
 * @brief      - Enables or disables the peripheral clock for the given GPIO port.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 * @param[in]  - EnOrDi : ENABLE or DISABLE.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi)
{
    if (EnOrDi)
    {
        if (pGPIOx == GPIOA)
        {
            GPIOA_PCLK_EN();
        }
        else if (pGPIOx == GPIOB)
        {
            GPIOB_PCLK_EN();
        }
        else if (pGPIOx == GPIOC)
        {
            GPIOC_PCLK_EN();
        }
        else if (pGPIOx == GPIOD)
        {
            GPIOD_PCLK_EN();
        }
        else if (pGPIOx == GPIOE)
        {
            GPIOE_PCLK_EN();
        }
        else if (pGPIOx == GPIOF)
        {
            GPIOF_PCLK_EN();
        }
        else if (pGPIOx == GPIOG)
        {
            GPIOG_PCLK_EN();
        }
        else if (pGPIOx == GPIOH)
        {
            GPIOH_PCLK_EN();
        }
        else if (pGPIOx == GPIOI)
        {
            GPIOI_PCLK_EN();
        }
    }
    else
    {
        if (pGPIOx == GPIOA)
        {
            GPIOA_PCLK_DI();
        }
        else if (pGPIOx == GPIOB)
        {
            GPIOB_PCLK_DI();
        }
        else if (pGPIOx == GPIOC)
        {
            GPIOC_PCLK_DI();
        }
        else if (pGPIOx == GPIOD)
        {
            GPIOD_PCLK_DI();
        }
        else if (pGPIOx == GPIOE)
        {
            GPIOE_PCLK_DI();
        }
        else if (pGPIOx == GPIOF)
        {
            GPIOF_PCLK_DI();
        }
        else if (pGPIOx == GPIOG)
        {
            GPIOG_PCLK_DI();
        }
        else if (pGPIOx == GPIOH)
        {
            GPIOH_PCLK_DI();
        }
        else if (pGPIOx == GPIOI)
        {
            GPIOI_PCLK_DI();
        }
    }
}


/************************************************************************************
 * @fn         - GPIO_Init
 *
 * @brief      - Initializes a GPIO pin according to the specified configuration.
 *
 * @param[in]  - pGPIOHandle : Pointer to the GPIO handle structure.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void GPIO_Init(GPIO_Handle_t *pGPIOHandle)
{
    uint32_t temp = 0;

    // Enable the peripheral clock
    GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);


    // 1. Configure the mode of GPIO pin
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG)
    {
        // Non-interrupt mode

        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode
                << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

        pGPIOHandle->pGPIOx->MODER &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); // Clear
        pGPIOHandle->pGPIOx->MODER |= temp; // Set
        temp = 0;
    }
    else
    {
        if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT)
        {
            // 1. Configure the FTSR
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

            // Clear the corresponding RTSR bit
            EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }
        else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT)
        {
            // 2. Configure the RTSR
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

            // Clear the corresponding FTSR bit
            EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }
        else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT)
        {
            // 3. Configure both FTSR and RTSR
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }

        // 2. Configure the GPIO port selection in SYSCFG_EXTICR
        uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
        uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;

        uint8_t portcode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);

        SYSCFG_PCLK_EN();
		SYSCFG->EXTICR[temp1] &= ~(0xFU << (temp2 * 4));               // Clear
		SYSCFG->EXTICR[temp1] |=  ((uint32_t)portcode << (temp2 * 4)); // Set

        // 3. Configure the EXTI interrupt delivery using IMR
        EXTI->IMR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
    }

    temp = 0;

    // 2. Configure the speed
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed
            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); // Clear
    pGPIOHandle->pGPIOx->OSPEEDR |= temp; // Set

    temp = 0;

    // 3. Configure the pull-up/pull-down settings
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl
            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    pGPIOHandle->pGPIOx->PUPDR &= ~(0x03 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); // Clear
    pGPIOHandle->pGPIOx->PUPDR |= temp; // Set

    temp = 0;

    // 4. Configure the output type
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType
            << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);

    pGPIOHandle->pGPIOx->OTYPER &= ~(0x01 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber); // Clear
    pGPIOHandle->pGPIOx->OTYPER |= temp; // Set

    temp = 0;

    // 5. Configure the alternate functionality
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)
    {
        // Configure the alternate function registers
        uint8_t temp1, temp2;

        temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
        temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;

        pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2)); // Clear
        pGPIOHandle->pGPIOx->AFR[temp1] |=
            (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4 * temp2)); // Set
    }
}


/************************************************************************************
 * @fn         - GPIO_DeInit
 *
 * @brief      - Resets the specified GPIO peripheral to its default reset state.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx)
{
    if (pGPIOx == GPIOA)
    {
        GPIOA_REG_RESET();
    }
    else if (pGPIOx == GPIOB)
    {
        GPIOB_REG_RESET();
    }
    else if (pGPIOx == GPIOC)
    {
        GPIOC_REG_RESET();
    }
    else if (pGPIOx == GPIOD)
    {
        GPIOD_REG_RESET();
    }
    else if (pGPIOx == GPIOE)
    {
        GPIOE_REG_RESET();
    }
    else if (pGPIOx == GPIOF)
    {
        GPIOF_REG_RESET();
    }
    else if (pGPIOx == GPIOG)
    {
        GPIOG_REG_RESET();
    }
    else if (pGPIOx == GPIOH)
    {
        GPIOH_REG_RESET();
    }
    else if (pGPIOx == GPIOI)
    {
        GPIOI_REG_RESET();
    }
}



/************************************************************************************
 * @fn         - GPIO_ReadFromInputPin
 *
 * @brief      - Reads the logic level of the specified GPIO input pin.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 * @param[in]  - PinNumber : GPIO pin number.
 *
 * @return     - GPIO pin state (SET or RESET).
 *
 * @note       - None.
 ************************************************************************************/
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
	uint8_t value;

	value = (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x00000001);

	return value;
}


/***********************************************************************************
 * @fn         - GPIO_ReadFromInputPort
 *
 * @brief      - Reads the logic levels of all GPIO pins in the specified port.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 *
 * @return     - 16-bit value representing the state of all GPIO pins.
 *
 * @note       - None.
 ************************************************************************************/
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx)
{
    uint16_t value;

    value = (uint16_t)pGPIOx->IDR;

    return value;
}


/***************************************************************************
 * @fn         - GPIO_WriteToOutputPin
 *
 * @brief      - Writes a logic value to the specified GPIO output pin.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 * @param[in]  - PinNumber : GPIO pin number.
 * @param[in]  - Value : GPIO_PIN_SET or GPIO_PIN_RESET.
 *
 * @return     - None.
 *
 * @note       - None.
 ***************************************************************************/
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx,
                           uint8_t PinNumber,
                           uint8_t Value)
{
    if (Value == GPIO_PIN_SET)
    {
        // Write 1 to the output data register at the bit position
        // corresponding to the selected pin.
        pGPIOx->ODR |= (1 << PinNumber);
    }
    else
    {
        // Write 0 to the output data register at the bit position
        // corresponding to the selected pin.
        pGPIOx->ODR &= ~(1 << PinNumber);
    }
}


/***************************************************************************
 * @fn         - GPIO_WriteToOutputPort
 *
 * @brief      - Writes a 16-bit value to the GPIO output port.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 * @param[in]  - Value : 16-bit value to be written to the output data register.
 *
 * @return     - None.
 *
 * @note       - None.
 ***************************************************************************/
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value)
{
	pGPIOx->ODR = Value;
}


/***************************************************************************
 * @fn         - GPIO_ToggleOutputPin
 *
 * @brief      - Toggles the current state of the specified GPIO output pin.
 *
 * @param[in]  - pGPIOx : Base address of the GPIO peripheral.
 * @param[in]  - PinNumber : GPIO pin number.
 *
 * @return     - None.
 *
 * @note       - None.
 ***************************************************************************/
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber)
{
	pGPIOx->ODR ^= (1 << PinNumber);
}


/***************************************************************************
 * @fn         - GPIO_IRQConfig
 *
 * @brief      - Configures the GPIO interrupt settings.
 *
 * @param[in]  - IRQNumber : IRQ number to configure.
 * @param[in]  - EnOrDi : ENABLE or DISABLE.
 *
 * @return     - None.
 *
 * @note       - None.
 ***************************************************************************/
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    if(EnOrDi == ENABLE)
    {
        if(IRQNumber <= 31)
        {
            // Program ISER0 register
            *NVIC_ISER0 = (1U << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64) // 32 to 63
        {
            // Program ISER1 register
            *NVIC_ISER1 = (1U << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96) // 64 to 95
        {
            // Program ISER2 register
            *NVIC_ISER2 = (1U << (IRQNumber % 64));
        }
    }
    else
    {
        if(IRQNumber <= 31)
        {
            // Program ICER0 register
            *NVIC_ICER0 = (1U << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64) // 32 to 63
        {
            // Program ICER1 register
            *NVIC_ICER1 = (1U << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96) // 64 to 95
        {
            // Program ICER2 register
            *NVIC_ICER2 = (1U << (IRQNumber % 64));
        }
    }
}


/***************************************************************************
 * @fn         - GPIO_IRQPriorityConfig
 *
 * @brief      - Configures the priority of a given IRQ number.
 *
 * @param[in]  - IRQNumber  : IRQ number.
 * @param[in]  - IRQPriority: Priority value for the IRQ.
 *
 * @return     - None.
 *
 * @note       - None.
  ***************************************************************************/
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    // 1. First let's find out the IPR register
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount =
        (8 * iprx_section) + (8 - NVIC_PR_BITS_IMPLEMENTED);
	
	*(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFU << shift_amount);                       // Clear
	*(NVIC_PR_BASE_ADDR + iprx) |=  ((IRQPriority & 0xFU) << shift_amount);       // Set
}

/***************************************************************************
 * @fn         - GPIO_IRQHandling
 *
 * @brief      - Handles the interrupt generated by the specified GPIO pin.
 *
 * @param[in]  - PinNumber : GPIO pin number that generated the interrupt.
 *
 * @return     - None.
 *
 * @note       - Call this function from the corresponding interrupt service routine (ISR).
 ***************************************************************************/
void GPIO_IRQHandling(uint8_t PinNumber)
{
    // Clear the EXTI PR register corresponding to the pin number
    if (EXTI->PR & (1U << PinNumber))
    {
        // Clear
        EXTI->PR = (1U << PinNumber);
    }
}

