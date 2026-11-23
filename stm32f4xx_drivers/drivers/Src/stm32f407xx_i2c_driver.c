/*
 * stm32f407xx_i2c_driver.c
 *
 *  Created on: 24 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */

#include "stm32f407xx_i2c_driver.h"

uint16_t AHB_PreScaler[8]  = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t  APB1_PreScaler[4] = {2, 4, 8, 16};

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_RegDef_t *pI2Cx);
static void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);

/************************************************************************************
 * @fn         - I2C_GenerateStartCondition
 *
 * @brief      - Generates the START condition on the I2C bus.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 *
 * @return     - None.
 *
 * @note       - Private helper function.
 ************************************************************************************/
static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1 << I2C_CR1_START);
}

/************************************************************************************
 * @fn         - I2C_ClearADDRFlag
 *
 * @brief      - Clears the ADDR flag by reading SR1 and then SR2.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 *
 * @return     - None.
 *
 * @note       - Private helper function.
 ************************************************************************************/
static void I2C_ClearADDRFlag(I2C_RegDef_t *pI2Cx)
{
    uint32_t dummyRead;

    dummyRead = pI2Cx->SR1;
    dummyRead = pI2Cx->SR2;
    (void)dummyRead;
}


/************************************************************************************
 * @fn         - I2C_ExecuteAddressPhase
 *
 * @brief      - Sends the slave address with the R/nW bit set to write (0).
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - SlaveAddr: 7-bit address of the slave.
 *
 * @return     - None.
 *
 * @note       - Private helper function.
 ************************************************************************************/
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    SlaveAddr = SlaveAddr << 1;   // Make space for the R/nW bit
    SlaveAddr &= ~(1);            // The LSB is the R/nW bit: it must be 0 for WRITE
    pI2Cx->DR = SlaveAddr;
}

/************************************************************************************
 * @fn         - I2C_GenerateStopCondition
 *
 * @brief      - Generates the STOP condition on the I2C bus.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 *
 * @return     - None.
 *
 * @note       - Private helper function.
 ************************************************************************************/
static void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
    pI2Cx->CR1 |= (1 << I2C_CR1_STOP);
}

/************************************************************************************
 * @fn         - I2C_ExecuteAddressPhaseRead
 *
 * @brief      - Sends the slave address with the R/nW bit set to read (1).
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - SlaveAddr: 7-bit address of the slave.
 *
 * @return     - None.
 *
 * @note       - Private helper function.
 ************************************************************************************/
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
    SlaveAddr = SlaveAddr << 1;   // Make space for the R/nW bit
    SlaveAddr |= 1;               // The LSB is the R/nW bit: it must be 1 for READ
    pI2Cx->DR = SlaveAddr;
}

/************************************************************************************
 * @fn         - I2C_PeriClockControl
 *
 * @brief      - Enables or disables the peripheral clock for the given I2C peripheral.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - EnOrDi: Enable or disable the peripheral clock.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pI2Cx == I2C1)       { I2C1_PCLK_EN(); }
		else if (pI2Cx == I2C2)  { I2C2_PCLK_EN(); }
		else if (pI2Cx == I2C3)  { I2C3_PCLK_EN(); }
	}
	else
	{
		if (pI2Cx == I2C1)       { I2C1_PCLK_DI(); }
		else if (pI2Cx == I2C2)  { I2C2_PCLK_DI(); }
		else if (pI2Cx == I2C3)  { I2C3_PCLK_DI(); }
	}

}

/************************************************************************************
 * @fn         - RCC_GetPLLOutputClock
 *
 * @brief      - Returns the PLL output clock (not implemented, the PLL is not used)
 *
 * @return     - 0
 ************************************************************************************/
uint32_t RCC_GetPLLOutputClock(void)
{
    return 0;
}

/************************************************************************************
 * @fn         - RCC_GetPCLK1Value
 *
 * @brief      - Calculates the value of the APB1 clock (PCLK1)
 *
 * @return     - PCLK1 in Hz
 ************************************************************************************/
uint32_t RCC_GetPCLK1Value(void)
{
    uint32_t pclk1, SystemClk;
    uint16_t ahbp;
    uint8_t  clksrc, temp, apb1p;

    // 1. Find the system clock source
    clksrc = ((RCC->CFGR >> 2) & 0x3);

    if (clksrc == 0)        // HSI
    {
        SystemClk = 16000000;
    }
    else if (clksrc == 1)   // HSE
    {
        SystemClk = 8000000;
    }
    else if (clksrc == 2)   // PLL
    {
        SystemClk = RCC_GetPLLOutputClock();
    }

    // 2. Find the AHB prescaler
    temp = ((RCC->CFGR >> 4) & 0xF);

    if (temp < 8)
    {
        ahbp = 1;
    }
    else
    {
        ahbp = AHB_PreScaler[temp - 8];
    }

    // 3. Find the APB1 prescaler
    temp = ((RCC->CFGR >> 10) & 0x7);

    if (temp < 4)
    {
        apb1p = 1;
    }
    else
    {
        apb1p = APB1_PreScaler[temp - 4];
    }

    pclk1 = (SystemClk / ahbp) / apb1p;

    return pclk1;
}

/************************************************************************************
 * @fn         - I2C_Init
 *
 * @brief      - Initializes the I2C peripheral according to the user configuration.
 *
 * @param[in]  - pI2CHandle: Pointer to the I2C handle containing the I2C peripheral
 *                           and its configuration.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_Init(I2C_Handle_t *pI2CHandle)
{
    uint32_t tempreg = 0;

    // 1. Enable the peripheral clock
    I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

    // 2. ACK control
    tempreg |= pI2CHandle->I2C_Config.I2C_ACKControl << I2C_CR1_ACK;
    pI2CHandle->pI2Cx->CR1 = tempreg;

    // 3. Configure the FREQ field of CR2
    tempreg = 0;
    tempreg |= RCC_GetPCLK1Value() / 1000000U;
    pI2CHandle->pI2Cx->CR2 = (tempreg & 0x3F);

    // 4. Program the device own address
    tempreg = 0;
    tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
    tempreg |= (1 << 14);
    pI2CHandle->pI2Cx->OAR1 = tempreg;

    // 5. CCR calculation
    uint16_t ccr_value = 0;
    tempreg = 0;

    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        // Standard mode
        ccr_value = RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
        tempreg |= (ccr_value & 0xFFF);
    }
    else
    {
        // Fast mode

        // Select fast mode
        tempreg |= (1 << I2C_CCR_FS);

        // Select the duty cycle
        tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << I2C_CCR_DUTY);

        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
        {
            ccr_value = RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
        }
        else
        {
            ccr_value = RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed);
        }

        tempreg |= (ccr_value & 0xFFF);
    }

    pI2CHandle->pI2Cx->CCR = tempreg;

    // 6. TRISE configuration
    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
    {
        // Standard mode: tr(max) = 1000 ns
        tempreg = (RCC_GetPCLK1Value() / 1000000U) + 1;
    }
    else
    {
        // Fast mode: tr(max) = 300 ns
        tempreg = ((RCC_GetPCLK1Value() / 1000000U) * 300U) / 1000U + 1;
    }

    pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3F);   // TRISE[5:0]: 6 valid bits

}

/************************************************************************************
 * @fn         - I2C_MasterSendData
 *
 * @brief      - Sends data from the I2C master to a slave.
 *
 * @param[in]  - pI2CHandle: Pointer to the I2C handle.
 * @param[in]  - pTxBuffer: Pointer to the data to send.
 * @param[in]  - Len: Number of bytes to send.
 * @param[in]  - SlaveAddr: 7-bit address of the slave.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    // 1. Generate the START condition
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // 2. Confirm that the start generation is completed by checking the SB flag in SR1
    //    Note: until SB is cleared, SCL is stretched (pulled LOW)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

    // 3. Send the address of the slave with the R/nW bit set to write (0) (total 8 bits)
    I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

    // 4. Confirm that the address phase is completed by checking the ADDR flag in SR1
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));

    // 5. Clear the ADDR flag according to its software sequence
    //    Note: until ADDR is cleared, SCL is stretched (pulled LOW)
    I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

    // 6. Send the data until Len becomes 0
    while (Len > 0)
    {
        // Wait until TXE is set
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));

        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        Len--;
    }

    // 7. When Len becomes 0, wait for TXE = 1 and BTF = 1 before generating the STOP condition
    //    Note: TXE = 1 and BTF = 1 mean that both the shift register and the data register
    //    are empty and the next transmission should begin.
    //    When BTF = 1, SCL is stretched (pulled LOW)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF));


    // 8. Generate the STOP condition. The master need not wait for the completion of the STOP condition.
    //    Note: generating STOP automatically clears BTF
    if (Sr == I2C_DISABLE_SR)
    {
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
    }
}

/************************************************************************************
 * @fn         - I2C_MasterReceiveData
 *
 * @brief      - Receives data from a slave into the I2C master.
 *
 * @param[in]  - pI2CHandle: Pointer to the I2C handle.
 * @param[in]  - pRxBuffer: Pointer to the buffer where the received data is stored.
 * @param[in]  - Len: Number of bytes to receive.
 * @param[in]  - SlaveAddr: 7-bit address of the slave.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
    // 1. Generate the START condition
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // 2. Confirm that the start generation is completed by checking the SB flag in SR1
    //    Note: until SB is cleared, SCL is stretched (pulled LOW)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

    // 3. Send the address of the slave with the R/nW bit set to read (1) (total 8 bits)
    I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

    // 4. Wait until the address phase is completed by checking the ADDR flag in SR1
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));

    // Procedure to read only 1 byte from the slave
    if (Len == 1)
    {
        // Disable ACKing
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

        // Clear the ADDR flag
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        // Wait until RXNE becomes 1
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

        // Generate the STOP condition
        if (Sr == I2C_DISABLE_SR)
        {
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        }

        // Read the data into the buffer
        *pRxBuffer = pI2CHandle->pI2Cx->DR;
    }

    // Procedure to read data from the slave when Len > 1
    if (Len > 1)
    {
        // Clear the ADDR flag
        I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

        // Read the data until Len becomes 0
        for (uint32_t i = Len; i > 0; i--)
        {
            // Wait until RXNE becomes 1
            while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

            if (i == 2) // If the last 2 bytes remain
            {
                // Clear the ACK bit
                I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

                // Generate the STOP condition
                if (Sr == I2C_DISABLE_SR)
                {
                    I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                }
            }

            // Read the data from the data register into the buffer
            *pRxBuffer = pI2CHandle->pI2Cx->DR;

            // Increment the buffer address
            pRxBuffer++;
        }
    }

    // Re-enable ACKing, if the user configured it
    if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE)
    {
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
    }
}

/************************************************************************************
 * @fn         - I2C_ManageAcking
 *
 * @brief      - Enables or disables ACKing (ACK bit of CR1).
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - EnOrDi: I2C_ACK_ENABLE or I2C_ACK_DISABLE.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == I2C_ACK_ENABLE)
    {
        // Enable the ACK
        pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
    }
    else
    {
        // Disable the ACK
        pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
    }
}
/************************************************************************************
 * @fn         - I2C_DeInit
 *
 * @brief      - De-initializes the I2C peripheral and resets its configuration.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if (pI2Cx == I2C1)
    {
        I2C1_REG_RESET();
    }
    else if (pI2Cx == I2C2)
    {
        I2C2_REG_RESET();
    }
    else if (pI2Cx == I2C3)
    {
        I2C3_REG_RESET();
    }
}

/************************************************************************************
 * @fn         - I2C_IRQInterruptConfig
 *
 * @brief      - Enables or disables the I2C interrupt in the NVIC.
 *
 * @param[in]  - IRQNumber: IRQ number of the I2C interrupt.
 * @param[in]  - EnOrDi: Enable or disable the interrupt.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (IRQNumber <= 31)
        {
            *NVIC_ISER0 |= (1 << IRQNumber);
        }
        else if (IRQNumber > 31 && IRQNumber < 64)    // 32 to 63
        {
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        }
        else if (IRQNumber >= 64 && IRQNumber < 96)   // 64 to 95
        {
            *NVIC_ISER2 |= (1 << (IRQNumber % 32));
        }
    }
    else
    {
        if (IRQNumber <= 31)
        {
            *NVIC_ICER0 |= (1 << IRQNumber);
        }
        else if (IRQNumber > 31 && IRQNumber < 64)
        {
            *NVIC_ICER1 |= (1 << (IRQNumber % 32));
        }
        else if (IRQNumber >= 64 && IRQNumber < 96)
        {
            *NVIC_ICER2 |= (1 << (IRQNumber % 32));
        }
    }
}

/************************************************************************************
 * @fn         - I2C_IRQPriorityConfig
 *
 * @brief      - Configures the priority of the I2C interrupt in the NVIC.
 *
 * @param[in]  - IRQNumber: IRQ number of the I2C interrupt.
 * @param[in]  - IRQPriority: Priority value assigned to the I2C interrupt.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
    // Find out the IPR register and the section inside it
    uint8_t iprx         = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount = (8 * iprx_section) + (8 - NVIC_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

/************************************************************************************
 * @fn         - I2C_GetFlagStatus
 *
 * @brief      - Returns the status of the requested I2C flag.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - FlagName: Flag to be checked.
 *
 * @return     - FLAG_SET if the flag is set, otherwise FLAG_RESET.
 *
 * @note       - None.
 ************************************************************************************/
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if (pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}

	return FLAG_RESET;
}

/************************************************************************************
 * @fn         - I2C_PeripheralControl
 *
 * @brief      - Enables or disables the I2C peripheral.
 *
 * @param[in]  - pI2Cx: Pointer to the I2C peripheral.
 * @param[in]  - EnOrDi: ENABLE or DISABLE.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        // Enable the I2C peripheral by setting the PE bit
        pI2Cx->CR1 |= (1 << I2C_CR1_PE);
    }
    else
    {
        // Disable the I2C peripheral by clearing the PE bit
        pI2Cx->CR1 &= ~(1 << I2C_CR1_PE);
    }
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv)
{

}

