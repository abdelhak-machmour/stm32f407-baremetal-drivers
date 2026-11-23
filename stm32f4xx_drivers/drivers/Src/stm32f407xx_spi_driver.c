/*
 * stm32f407xx_spi_driver.c
 *
 *  Created on: 10 Sep 2026
 *      Author: Abdelhak MACHMOUR
 */

#include "stm32f407xx_spi_driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);


/************************************************************************************
 * @fn         - SPI_PeriClockControl
 *
 * @brief      - Enables or disables the peripheral clock for the given SPI peripheral.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 * @param[in]  - EnOrDi: Enable or disable the peripheral clock.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        if (pSPIx == SPI1)
        {
            SPI1_PCLK_EN();
        }
        else if (pSPIx == SPI2)
        {
            SPI2_PCLK_EN();
        }
        else if (pSPIx == SPI3)
        {
            SPI3_PCLK_EN();
        }
        else if (pSPIx == SPI4)
        {
            SPI4_PCLK_EN();
        }
        else if (pSPIx == SPI5)
        {
            SPI5_PCLK_EN();
        }
        else if (pSPIx == SPI6)
        {
            SPI6_PCLK_EN();
        }
    }
    else
    {
        if (pSPIx == SPI1)
        {
            SPI1_PCLK_DI();
        }
        else if (pSPIx == SPI2)
        {
            SPI2_PCLK_DI();
        }
        else if (pSPIx == SPI3)
        {
            SPI3_PCLK_DI();
        }
        else if (pSPIx == SPI4)
        {
            SPI4_PCLK_DI();
        }
        else if (pSPIx == SPI5)
        {
            SPI5_PCLK_DI();
        }
        else if (pSPIx == SPI6)
        {
            SPI6_PCLK_DI();
        }
    }
}

/************************************************************************************
 * @fn         - SPI_Init
 *
 * @brief      - Initializes the SPI peripheral according to the user configuration.
 *
 * @param[in]  - pSPIHandle: Pointer to the SPI handle containing the SPI peripheral
 *                and its configuration.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	// Peripheral clock enable
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);

    // First let's configure the SPI_CR1

    uint32_t tempreg = 0;

    // 1. Configure the device mode

    tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode
               << SPI_CR1_MSTR;


    // 2. Configure the bus config

    if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
    {
        // BIDIMODE should be cleared

        tempreg &= ~(1 << SPI_CR1_BIDIMODE);
    }

    else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
    {
        // BIDIMODE should be set

        tempreg |= (1 << SPI_CR1_BIDIMODE);
    }

    else if (pSPIHandle->SPIConfig.SPI_BusConfig
             == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
    {
        // BIDIMODE should be cleared

        tempreg &= ~(1 << SPI_CR1_BIDIMODE);

        // RXONLY bit must be set

        tempreg |= (1 << SPI_CR1_RXONLY);
    }


    // 3. Configure the SPI serial clock speed (baud rate)

    tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed
               << SPI_CR1_BR;


    // 4. Configure the DFF

    tempreg |= pSPIHandle->SPIConfig.SPI_DFF
               << SPI_CR1_DFF;


    // 5. Configure the CPOL

    tempreg |= pSPIHandle->SPIConfig.SPI_CPOL
               << SPI_CR1_CPOL;


    // 6. Configure the CPHA

    tempreg |= pSPIHandle->SPIConfig.SPI_CPHA
               << SPI_CR1_CPHA;

    // 7. Configure the SSM
    tempreg |= pSPIHandle->SPIConfig.SPI_SSM
                   << SPI_CR1_SSM;

    pSPIHandle->pSPIx->CR1 = tempreg;
}

/************************************************************************************
 * @fn         - SPI_DeInit
 *
 * @brief      - De-initializes the SPI peripheral and resets its configuration.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
    if (pSPIx == SPI1)
    {
        SPI1_REG_RESET();
    }
    else if (pSPIx == SPI2)
    {
        SPI2_REG_RESET();
    }
    else if (pSPIx == SPI3)
    {
        SPI3_REG_RESET();
    }
    else if (pSPIx == SPI4)
    {
        SPI4_REG_RESET();
    }
    else if (pSPIx == SPI5)
    {
        SPI5_REG_RESET();
    }
    else if (pSPIx == SPI6)
    {
        SPI6_REG_RESET();
    }
}

/************************************************************************************
 * @fn         - SPI_GetFlagStatus
 *
 * @brief      - Returns the status of the requested SPI flag.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 * @param[in]  - FlagName: Flag to be checked.
 *
 * @return     - FLAG_SET if the flag is set, otherwise FLAG_RESET.
 *
 * @note       - None.
 ************************************************************************************/
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
    if (pSPIx->SR & FlagName)
    {
        return FLAG_SET;
    }

    return FLAG_RESET;
}

/************************************************************************************
 * @fn         - SPI_PeripheralControl
 *
 * @brief      - Enables or disables the SPI peripheral.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 * @param[in]  - EnOrDi: ENABLE or DISABLE.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        // Enable the SPI peripheral by setting the SPE bit
        pSPIx->CR1 |= (1 << SPI_CR1_SPE);
    }
    else
    {
        // Disable the SPI peripheral by clearing the SPE bit
        pSPIx->CR1 &= ~(1 << SPI_CR1_SPE);
    }
}

/************************************************************************************
 * @fn      - SPI_SSIConfig
 *
 * @brief   - Enables or disables the SSI bit (internal NSS level)
 *
 * @param[in] pSPIx  : Base address of the SPI peripheral
 * @param[in] EnOrDi : ENABLE or DISABLE
 *
 * @return  - None
 *
 * @note    - Only meaningful when SSM = 1
 ************************************************************************************/
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        // Set SSI to keep the internal NSS signal high
        pSPIx->CR1 |= (1 << SPI_CR1_SSI);
    }
    else
    {
        // Clear SSI to make the internal NSS signal low
        pSPIx->CR1 &= ~(1 << SPI_CR1_SSI);
    }
}

/************************************************************************************
 * @fn      - SPI_SSOEConfig
 *
 * @brief   - Enables or disables the SSOE bit (NSS output enable)
 *
 * @param[in] pSPIx  : Base address of the SPI peripheral
 * @param[in] EnOrDi : ENABLE or DISABLE
 *
 * @return  - None
 *
 * @note    - Used with SSM = 0 when the device is master
 ************************************************************************************/
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
    if (EnOrDi == ENABLE)
    {
        pSPIx->CR2 |= (1 << SPI_CR2_SSOE);
    }
    else
    {
        pSPIx->CR2 &= ~(1 << SPI_CR2_SSOE);
    }
}

/************************************************************************************
 * @fn         - SPI_SendData
 *
 * @brief      - Sends data through the SPI peripheral using a blocking call.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 * @param[in]  - pTxBuffer: Pointer to the buffer containing the data to transmit.
 * @param[in]  - Len: Length of the data to transmit in bytes.
 *
 * @return     - None.
 *
 * @note       - This is a blocking API.
 ************************************************************************************/
 void SPI_SendData(SPI_RegDef_t *pSPIx,
                   uint8_t *pTxBuffer,
                   uint32_t Len)
 {
	 while (Len > 0)
	 {
		 // 1. Wait until TXE is set
		 while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET);

		 // 2. Check the DFF bit in CR1
		 if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
		 {
			 // 16-bit DFF

			 // Load 2 bytes of data into the DR
			 pSPIx->DR = *((uint16_t *)pTxBuffer);

			 // We have loaded 2 bytes of data
			 Len--;
			 Len--;

			 // Move the buffer pointer to the next 2 bytes
			 pTxBuffer += 2;
		 }
		 else
		 {
			 // 8-bit DFF

			 // Load 1 byte of data into the DR
			 pSPIx->DR = *pTxBuffer;

			 // We have loaded 1 byte of data
			 Len--;

			 // Move the buffer pointer to the next byte
			 pTxBuffer++;
		 }
	 }

 }


/************************************************************************************
 * @fn         - SPI_ReceiveData
 *
 * @brief      - Receives data from the SPI peripheral using a blocking call.
 *
 * @param[in]  - pSPIx: Pointer to the SPI peripheral.
 * @param[in]  - pRxBuffer: Pointer to the buffer where received data will be stored.
 * @param[in]  - Len: Length of the data to receive in bytes.
 *
 * @return     - None.
 *
 * @note       - This is a blocking API.
 ************************************************************************************/
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,
                     uint8_t *pRxBuffer,
                     uint32_t Len)
{
    while (Len > 0)
    {
        // 1. Wait until RXNE is set
        while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

        // 2. Check the DFF bit in CR1
        if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
        {
            // 16-bit DFF
            // Load data from DR into the Rx buffer address
            *((uint16_t *)pRxBuffer) = pSPIx->DR;
            Len--;
            Len--;
            pRxBuffer += 2;   // Move to the next free memory address
        }
        else
        {
            // 8-bit DFF
            *pRxBuffer = pSPIx->DR;
            Len--;
            pRxBuffer++;      // Move to the next free memory address
        }
    }
}


/************************************************************************************
 * @fn         - SPI_IRQInterruptConfig
 *
 * @brief      - Enables or disables the SPI interrupt in the NVIC.
 *
 * @param[in]  - IRQNumber: IRQ number of the SPI interrupt.
 * @param[in]  - EnOrDi: Enable or disable the interrupt.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
    {
        if(IRQNumber <= 31)
        {
            // Program ISER0 register
            *NVIC_ISER0 |= (1 << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64) // 32 to 63
        {
            // Program ISER1 register
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96) // 64 to 95
        {
            // Program ISER2 register
            *NVIC_ISER2 |= (1 << (IRQNumber % 64));
        }
    }
    else
    {
        if(IRQNumber <= 31)
        {
            // Program ICER0 register
            *NVIC_ICER0 |= (1 << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64) // 32 to 63
        {
            // Program ICER1 register
            *NVIC_ICER1 |= (1 << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96) // 64 to 95
        {
            // Program ICER2 register
            *NVIC_ICER2 |= (1 << (IRQNumber % 64));
        }
    }

}


/************************************************************************************
 * @fn         - SPI_IRQPriorityConfig
 *
 * @brief      - Configures the priority of the SPI interrupt in the NVIC.
 *
 * @param[in]  - IRQNumber: IRQ number of the SPI interrupt.
 * @param[in]  - IRQPriority: Priority value assigned to the SPI interrupt.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_IRQPriorityConfig(uint8_t IRQNumber,
                           uint32_t IRQPriority)
{
	// Find out the IPR register and the section inside it
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount =
        (8 * iprx_section) + (8 - NVIC_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + (iprx)) |= (IRQPriority << shift_amount);

}

/************************************************************************************
 * @fn         - SPI_SendDataIT
 *
 * @brief      - Sends data through the SPI peripheral using interrupt mode.
 *
 * @param[in]  - pSPIHandle: Pointer to the handle structure.
 * @param[in]  - pTxBuffer: Pointer to the buffer containing the data to transmit.
 * @param[in]  - Len: Length of the data to transmit in bytes.
 *
 * @return     - The Tx state of the SPI before the call.
 *
 * @note       - This is a non-blocking API.
 ************************************************************************************/
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->TxState;

    // Do all the following only if the SPI is free (not already busy in transmission)
    if (state != SPI_BUSY_IN_TX)
    {
        // 1. Save the Tx buffer address and Len information in the handle
        pSPIHandle->pTxBuffer = pTxBuffer;
        pSPIHandle->TxLen     = Len;

        // 2. Mark the SPI state as busy in transmission, so that no other code
        //    can take over the same SPI peripheral until the transmission is over
        pSPIHandle->TxState = SPI_BUSY_IN_TX;

        // 3. Enable the TXEIE control bit to get an interrupt whenever
        //    the TXE flag is set in SR
        pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);
    }

    // 4. Data transmission is handled by the ISR code

    return state;
}


/************************************************************************************
 * @fn         - SPI_ReceiveDataIT
 *
 * @brief      - Receives data from the SPI peripheral using interrupt mode.
 *
 * @param[in]  - pSPIHandle: Pointer to the handle structure.
 * @param[in]  - pRxBuffer: Pointer to the buffer where received data will be stored.
 * @param[in]  - Len: Length of the data to receive in bytes.
 *
 * @return     - The Rx state of the SPI before the call.
 *
 * @note       - This is a non-blocking API.
 ************************************************************************************/
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
    uint8_t state = pSPIHandle->RxState;

    // Do all the following only if the SPI is free (not already busy in reception)
    if (state != SPI_BUSY_IN_RX)
    {
        // 1. Save the Rx buffer address and Len information in the handle
        pSPIHandle->pRxBuffer = pRxBuffer;
        pSPIHandle->RxLen     = Len;

        // 2. Mark the SPI state as busy in reception, so that no other code
        //    can take over the same SPI peripheral until the reception is over
        pSPIHandle->RxState = SPI_BUSY_IN_RX;

        // 3. Enable the RXNEIE control bit to get an interrupt whenever
        //    the RXNE flag is set in SR
        pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);
    }

    // 4. Data reception is handled by the ISR code (implemented later)

    return state;
}

/************************************************************************************
 * @fn         - SPI_IRQHandling
 *
 * @brief      - Handles SPI interrupts and processes the corresponding events.
 *
 * @param[in]  - pHandle: Pointer to the SPI handle.
 *
 * @return     - None.
 *
 * @note       - None.
 ************************************************************************************/
void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
    uint8_t temp1, temp2;

    // First, check for TXE
    temp1 = pHandle->pSPIx->SR  & (1 << SPI_SR_TXE);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);

    if (temp1 && temp2)
    {
        // Handle TXE
        spi_txe_interrupt_handle(pHandle);
    }

    // Check for RXNE
    temp1 = pHandle->pSPIx->SR  & (1 << SPI_SR_RXNE);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);

    if (temp1 && temp2)
    {
        // Handle RXNE
        spi_rxne_interrupt_handle(pHandle);
    }

    // Check for the overrun error (OVR)
    temp1 = pHandle->pSPIx->SR  & (1 << SPI_SR_OVR);
    temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);

    if (temp1 && temp2)
    {
        // Handle the overrun error
        spi_ovr_err_interrupt_handle(pHandle);
    }
}

// Helper function implementation
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    // Check the DFF bit in CR1
    if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
    {
        // 16-bit DFF

        // Load 2 bytes of data into the DR
        pSPIHandle->pSPIx->DR = *((uint16_t *)pSPIHandle->pTxBuffer);

        // We have loaded 2 bytes of data
        pSPIHandle->TxLen--;
        pSPIHandle->TxLen--;

        // Move the buffer pointer to the next 2 bytes
        pSPIHandle->pTxBuffer += 2;
    }
    else
    {
        // 8-bit DFF

        // Load 1 byte of data into the DR
        pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;

        // We have loaded 1 byte of data
        pSPIHandle->TxLen--;

        // Move the buffer pointer to the next byte
        pSPIHandle->pTxBuffer++;
    }

    if (!pSPIHandle->TxLen)
    {
        // TxLen is 0: close the SPI transmission and inform the application that Tx is over.
    	SPI_CloseTransmission(pSPIHandle);

    }
}


static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    // Check the DFF bit in CR1
    if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
    {
        // 16-bit DFF

        // Load data from DR into the Rx buffer address
        *((uint16_t *)pSPIHandle->pRxBuffer) = (uint16_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen -= 2;
        pSPIHandle->pRxBuffer += 2;   // Move to the next free memory address
    }
    else
    {
        // 8-bit DFF
        *pSPIHandle->pRxBuffer = (uint8_t)pSPIHandle->pSPIx->DR;
        pSPIHandle->RxLen--;
        pSPIHandle->pRxBuffer++;      // Move to the next free memory address
    }

    if (!pSPIHandle->RxLen)
    {
        // RxLen is 0: close the SPI reception and inform the application that Rx is over
        SPI_CloseReception(pSPIHandle);
    }
}


static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
    uint8_t temp;

    // 1. Clear the OVR flag
    // This is done only if the SPI is not busy in transmission,
    // because the application may still need the data
    if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
    {
        temp = pSPIHandle->pSPIx->DR;
        temp = pSPIHandle->pSPIx->SR;
    }
    (void)temp;   // Avoid the "unused variable" warning

    // 2. Inform the application
    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
    // This prevents further interrupts from the TXE flag
    pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
    pSPIHandle->pTxBuffer = NULL;
    pSPIHandle->TxLen     = 0;
    pSPIHandle->TxState   = SPI_READY;

    // Inform the application
    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
    // This prevents further interrupts from the RXNE flag
    pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_RXNEIE);
    pSPIHandle->pRxBuffer = NULL;
    pSPIHandle->RxLen     = 0;
    pSPIHandle->RxState   = SPI_READY;

    // Inform the application
    SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
    uint8_t temp;

    temp = pSPIx->DR;
    temp = pSPIx->SR;
    (void)temp;
}

__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    // This is a weak implementation. The application may override this function.
}

