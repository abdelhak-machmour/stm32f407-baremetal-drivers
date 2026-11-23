# stm32f4xx_drivers

Driver library and test applications for the STM32F407VGT6.

```
stm32f4xx_drivers/
├── drivers/
│   ├── Inc/
│   │   ├── stm32f407xx.h              # MCU-specific header: base addresses, register structures,
│   │   │                              # clock enable/disable macros, IRQ numbers, bit positions
│   │   ├── stm32f407xx_gpio_driver.h
│   │   ├── stm32f407xx_spi_driver.h
│   │   └── stm32f407xx_i2c_driver.h
│   └── Src/
│       ├── stm32f407xx_gpio_driver.c
│       ├── stm32f407xx_spi_driver.c
│       └── stm32f407xx_i2c_driver.c
├── Src/                               # Test applications (one main() each) + syscalls (ITM printf)
└── Startup/                           # Vector table and reset handler
```

## Driver APIs

### GPIO

| Function | Role |
|---|---|
| `GPIO_PeriClockControl()` | Enable / disable the port clock (RCC AHB1ENR) |
| `GPIO_Init()` / `GPIO_DeInit()` | Configure a pin from a `GPIO_Handle_t` / reset the port |
| `GPIO_ReadFromInputPin()` / `GPIO_ReadFromInputPort()` | Read a pin / the whole port (IDR) |
| `GPIO_WriteToOutputPin()` / `GPIO_WriteToOutputPort()` | Write a pin / the whole port (ODR) |
| `GPIO_ToggleOutputPin()` | Toggle a pin |
| `GPIO_IRQInterruptConfig()` / `GPIO_IRQPriorityConfig()` | Enable an IRQ in the NVIC / set its priority |
| `GPIO_IRQHandling()` | Clear the EXTI pending bit, to call from the `EXTIx_IRQHandler` |

### SPI

| Function | Role |
|---|---|
| `SPI_PeriClockControl()`, `SPI_Init()`, `SPI_DeInit()` | Clock, configuration (`SPI_Config_t`), reset |
| `SPI_PeripheralControl()` | Enable / disable the peripheral (SPE) |
| `SPI_SSIConfig()` / `SPI_SSOEConfig()` | Software (SSI) or hardware (SSOE) NSS management |
| `SPI_SendData()` / `SPI_ReceiveData()` | Blocking transfers |
| `SPI_SendDataIT()` / `SPI_ReceiveDataIT()` | Interrupt-driven transfers, state kept in the handle |
| `SPI_IRQHandling()` | Dispatch TXE, RXNE and OVR events |
| `SPI_ApplicationEventCallback()` | Weak callback: `SPI_EVENT_TX_CMPLT`, `SPI_EVENT_RX_CMPLT`, `SPI_EVENT_OVR_ERR` |

### I2C

| Function | Role |
|---|---|
| `I2C_PeriClockControl()`, `I2C_Init()`, `I2C_DeInit()` | Clock, configuration (speed, ACK, own address, FM duty cycle), reset |
| `I2C_MasterSendData()` | Master transmit (START, address phase, data, optional STOP) |
| `I2C_MasterReceiveData()` | Master receive, with the 1-byte and N-byte ACK/STOP sequences |
| `I2C_ManageAcking()` | Enable / disable ACK |
| `I2C_IRQInterruptConfig()` / `I2C_IRQPriorityConfig()` | NVIC configuration |

The `Sr` parameter (`I2C_ENABLE_SR` / `I2C_DISABLE_SR`) keeps the bus with a **repeated START** instead of a STOP, which is needed for command/response exchanges.

## Test applications

| File | What it tests | Hardware |
|---|---|---|
| `001led_toggle.c` | GPIO output, toggling the green LED (PD12) | Board only |
| `002led_button.c` | GPIO input, external button on PB12 drives the LED | Button on PB12 |
| `003button_interrupt.c` | EXTI falling-edge interrupt on PD5, LED toggled in the ISR | Button on PD5 |
| `004spi_tx_testing.c` | SPI2 master transmit, signals checked with a logic analyser | Logic analyser |
| `005spi_txonly_arduino.c` | SPI2 master sends a string to an Arduino slave | Arduino Uno |
| `006spi_cmd_handling.c` | Command/response protocol over SPI (LED control, analog read, LED read, print, ID read) | Arduino Uno |
| `007spi_message_rcv_it.c` | Interrupt-driven SPI reception of a message typed in the Arduino serial monitor | Arduino Uno |
| `008spi_eeprom_interface_app.c` | **Personal extension:** storing flight data (name, ID, aircraft model, departure, destination, altitude, speed) in a 25LC256 SPI EEPROM. *Work in progress: memory map, SPI/LED/button setup and WREN/WRDI commands done; read/write sequence to come.* | 25LC256 EEPROM |
| `009i2C_master_tx_texting.c` | I2C1 master transmit to an Arduino slave on button press | Arduino Uno |
| `010i2c_master_rx_testing.c` | I2C1 master receive with repeated START (length request `0x51`, then data `0x52`) | Arduino Uno |

## Wiring

**SPI2** (alternate function 5)

| STM32 | Signal | Arduino Uno | 25LC256 |
|---|---|---|---|
| PB12 | NSS | D10 | CS |
| PB13 | SCLK | D13 | SCK |
| PB14 | MISO | D12 | SO |
| PB15 | MOSI | D11 | SI |
| GND | GND | GND | VSS |

**I2C1** (alternate function 4)

| STM32 | Signal | Arduino Uno |
|---|---|---|
| PB6 | SCL | A5 |
| PB9 | SDA | A4 |
| GND | GND | GND |

> The STM32 works at 3.3 V and the Arduino Uno at 5 V: use a logic level shifter on SPI lines. On I2C, use pull-up resistors to 3.3 V.

## Debug output

`printf` is redirected to the **ITM** (see `Src/syscalls.c`). In STM32CubeIDE, enable SWV in the debug configuration (core clock 16 MHz) and open `Window > Show View > SWV > SWV ITM Data Console`, port 0.
