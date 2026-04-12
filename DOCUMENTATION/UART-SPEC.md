# RCF Hardware: UART Connectivity Specification

## 1. Overview
The **UART Interface** provides a fallback connection for external debugging and auxiliary node-to-node telemetry.

## 2. Configuration
- **Baud Rate**: 115200 bps (Standard) / 921600 bps (High-Speed)
- **Parity**: None
- **Flow Control**: None (Software Pulse available)

## 3. Physical Pinout (STM32F407 Reference)
| Pin | Function | RCF Mapping |
| :--- | :--- | :--- |
| PA2 | USART2_TX | Telemetry Emit |
| PA3 | USART2_RX | Remote Listen |

## 4. A-VM Command Mapping
- `OP_LUME_VOICE (0xA0)` -> `HAL_UART_Transmit()`
- `OP_LUME_SUGGEST (0xA5)` -> `HAL_UART_Receive_IT()`

## 5. Security Compliance
Encryption is **NOT mandatory** for UART debug logs, but all telemetry packets MUST reach the RCF Security layer before further processing.
