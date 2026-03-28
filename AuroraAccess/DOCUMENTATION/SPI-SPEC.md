# RCF Hardware: SPI-Storage Specification

## 1. Overview
The **SPI-Storage** interface serves as a legacy or auxiliary connection for low-power sensors and small flash memory modules (e.g., Winbond W25 series).

## 2. Bus Configuration
- **Mode**: SPI Full-Duplex
- **Clock Speed**: 21MHz (Max for STM32F4 APB2)
- **Data Size**: 8-bit

## 3. Physical Pinout (STM32F407 Reference)
| Pin | Function | RCF Mapping |
| :--- | :--- | :--- |
| PA4 | SPI1_NSS | Chip Select |
| PA5 | SPI1_SCK | Serial Clock |
| PA6 | SPI1_MISO| Master In / Slave Out |
| PA7 | SPI1_MOSI| Master Out / Slave In |

## 4. A-VM Command Mapping
- `OP_BUS_PUB (0x20)` -> `HAL_SPI_Transmit()`
- `OP_BUS_SUB (0x21)` -> `HAL_SPI_Receive()`

## 5. Security Model
SPI communication is audited via the **Sentinel Trace** log. If a command attempts to bypass the security envelope, the `OP_INSTINCT_TRIGGER (0x60)` is automatically activated to isolate the bus.
