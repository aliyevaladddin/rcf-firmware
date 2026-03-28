# RCF Hardware Protocol v1.2.9

## 1. Overview
The **RCF Hardware Protocol** extends the Restricted Correlation Framework (RCF) to the machine level. It provides a standardized set of A-VM opcodes that allow signed modules to interact with physical peripherals (Flash cards, SD/MMC, SPI-Storage) without requiring hardware-specific drivers.

## 2. Security Model
Hardware access is governed by the RCF-PL license.
- **RESTRICTED**: Only modules with `EXT_HW_ACCESS` permission in their manifest can execute hardware opcodes.
- **AUDITED**: Every hardware mount and block operation is logged to the `rcf-audit` trail.
- **ENCRYPTED**: The firmware can optionally wrap hardware operations with `rcf-vault` encryption layers.

## 3. OpCode Specification (Range 0x30 - 0x3F)

| OpCode | Name | Payload | Description |
| :--- | :--- | :--- | :--- |
| `0x30` | `OP_EXT_MOUNT` | `[uint8 slot_id]` | Initialized and mounts the peripheral in the specified slot. |
| `0x31` | `OP_EXT_READ` | `[uint32 LBA, uint16 len]` | Reads blocks from the external device into the A-VM stack. |
| `0x32` | `OP_EXT_WRITE` | `[uint32 LBA, uint16 len]` | Writes data from the A-VM stack to the external device. |
| `0x35` | `OP_EXT_STATUS` | `None` | Returns `0x01` if device is connected, `0x00` otherwise. |
| `0x3F` | `OP_EXT_FORMAT` | `[uint8 fs_type]` | Erases and prepares the external media. |

## 4. Hardware Implementation (STM32F4)
On the STM32F407VG (Lume Reference Hardware):
- `OP_EXT_MOUNT` maps to `HAL_SD_Init()`.
- Data transfer utilizes DMA2 for maximum performance.
- Sector size is fixed at **512 bytes** for protocol consistency.

## 5. Compliance
All hardware-compliant drivers must include the RCF-PL v1.2.8 header and pass the `rcf-cli` integrity audit.
