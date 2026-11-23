/*
 * stm32f407xx_i2c_driver.h
 *
 *  Created on: 24 Oct 2026
 *      Author: Abdelhak MACHMOUR
 */

#ifndef INC_STM32F407XX_I2C_DRIVER_H_
#define INC_STM32F407XX_I2C_DRIVER_H_

#include "stm32f407xx.h"


/*
 * Configuration structure for the I2Cx peripheral
 */
typedef struct
{
    uint32_t I2C_SCLSpeed;       /*!< SCL speed (standard or fast mode)            */
    uint8_t  I2C_DeviceAddress;  /*!< Own address, used when the device is a slave */
    uint8_t  I2C_ACKControl;     /*!< Automatic ACK enabled or disabled            */
    uint16_t I2C_FMDutyCycle;    /*!< Clock duty cycle in fast mode                */
} I2C_Config_t;


/*
 * Handle structure for the I2Cx peripheral
 */
typedef struct
{
    I2C_RegDef_t *pI2Cx;         /*!< Base address of the I2Cx peripheral */
    I2C_Config_t  I2C_Config;    /*!< I2C configuration settings          */
} I2C_Handle_t;

/*
 * @I2C_SCLSpeed
 */
#define I2C_SCL_SPEED_SM      100000
#define I2C_SCL_SPEED_FM4K    400000
#define I2C_SCL_SPEED_FM2K    200000

/*
 * @I2C_ACKControl
 */
#define I2C_ACK_ENABLE     1
#define I2C_ACK_DISABLE    0

/*
 * @I2C_FMDutyCycle
 */
#define I2C_FM_DUTY_2       0
#define I2C_FM_DUTY_16_9    1

/********************************************************************************
 * Bit position definitions of the I2C peripheral
 ********************************************************************************/

/* Bit position definitions of I2C_CR1 */
#define I2C_CR1_PE          0
#define I2C_CR1_NOSTRETCH   7
#define I2C_CR1_START       8
#define I2C_CR1_STOP        9
#define I2C_CR1_ACK         10
#define I2C_CR1_SWRST       15

/* Bit position definitions of I2C_CR2 */
#define I2C_CR2_FREQ        0
#define I2C_CR2_ITERREN     8
#define I2C_CR2_ITEVTEN     9
#define I2C_CR2_ITBUFEN     10

/* Bit position definitions of I2C_OAR1 */
#define I2C_OAR1_ADD0       0
#define I2C_OAR1_ADD71      1
#define I2C_OAR1_ADD98      8
#define I2C_OAR1_ADDMODE    15

/* Bit position definitions of I2C_SR1 */
#define I2C_SR1_SB          0
#define I2C_SR1_ADDR        1
#define I2C_SR1_BTF         2
#define I2C_SR1_ADD10       3
#define I2C_SR1_STOPF       4
#define I2C_SR1_RXNE        6
#define I2C_SR1_TXE         7
#define I2C_SR1_BERR        8
#define I2C_SR1_ARLO        9
#define I2C_SR1_AF          10
#define I2C_SR1_OVR         11
#define I2C_SR1_TIMEOUT     14

/* Bit position definitions of I2C_SR2 */
#define I2C_SR2_MSL         0
#define I2C_SR2_BUSY        1
#define I2C_SR2_TRA         2
#define I2C_SR2_GENCALL     4
#define I2C_SR2_DUALF       7

/* Bit position definitions of I2C_CCR */
#define I2C_CCR_CCR         0
#define I2C_CCR_DUTY        14
#define I2C_CCR_FS          15


/*
 * I2C-related status flag definitions
 */
#define I2C_FLAG_SB        (1 << I2C_SR1_SB)
#define I2C_FLAG_ADDR      (1 << I2C_SR1_ADDR)
#define I2C_FLAG_BTF       (1 << I2C_SR1_BTF)
#define I2C_FLAG_STOPF     (1 << I2C_SR1_STOPF)
#define I2C_FLAG_RXNE      (1 << I2C_SR1_RXNE)
#define I2C_FLAG_TXE       (1 << I2C_SR1_TXE)
#define I2C_FLAG_BERR      (1 << I2C_SR1_BERR)
#define I2C_FLAG_ARLO      (1 << I2C_SR1_ARLO)
#define I2C_FLAG_AF        (1 << I2C_SR1_AF)
#define I2C_FLAG_OVR       (1 << I2C_SR1_OVR)
#define I2C_FLAG_TIMEOUT   (1 << I2C_SR1_TIMEOUT)

#define I2C_DISABLE_SR    RESET
#define I2C_ENABLE_SR     SET


/******************************************************************************************
 *                          APIs supported by this driver
 *          For more information about the APIs, check the function definitions.
 ******************************************************************************************/

/*
 * Peripheral clock setup
 */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

/*
 * Initialization and de-initialization
 */
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

/*
 * Data send and receive
 */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);


/*
 * IRQ configuration and ISR handling
 */
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);

/*
 * Other peripheral control APIs
 */
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName);
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);
void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

/*
 * Application callback
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv);


#endif /* INC_STM32F407XX_I2C_DRIVER_H_ */
