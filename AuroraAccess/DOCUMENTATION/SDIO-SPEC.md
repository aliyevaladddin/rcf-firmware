# RCF Hardware: SDIO Connectivity Specification

## 1. Overview
This document defines the high-speed **SDIO (Secure Digital Input Output)** interface for the RCF Hardware Protocol. It is used for primary filesystem storage and firmware module expansion.

## 2. Bus Configuration
- **Mode**: 4-bit Wide Bus
- **Clock Speed**: 24MHz (Nominal) / 48MHz (High-Speed)
- **Transfer Mode**: DMA2 (Direct Memory Access) Stream 3/6

## 3. Physical Pinout (STM32F407 Reference)
| Pin | Function | RCF Mapping |
| :--- | :--- | :--- |
| PC8 | SDIO_D0 | Data Lane 0 |
| PC9 | SDIO_D1 | Data Lane 1 |
| PC10 | SDIO_D2 | Data Lane 2 |
| PC11 | SDIO_D3 | Data Lane 3 |
| PC12 | SDIO_CK | Clock |
| PD2 | SDIO_CMD | Command |

## 4. A-VM Command Mapping
- `OP_EXT_MOUNT (0x30)` -> `HAL_SD_Init()`
- `OP_EXT_READ (0x31)` -> `HAL_SD_ReadBlocks_DMA()`
- `OP_EXT_WRITE (0x32)` -> `HAL_SD_WriteBlocks_DMA()`

## 5. Security Compliance
All data transmitted via SDIO is subject to **rcf-vault** block-level encryption when the `VAULT_ENFORCE` flag is active in the A-VM context.
