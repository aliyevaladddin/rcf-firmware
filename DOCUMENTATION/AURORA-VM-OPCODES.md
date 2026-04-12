# Aurora Virtual Machine (A-VM) Opcode Reference (v1.3)

[ [RCF:PROTECTED] ]

This document provides the technical specification for the A-VM bytecode instruction set used in Aurora Access `.acode` modules.

## 1. Module Lifecycle

| Opcode | Mnemonic | Description |
|:---:|---|---|
| `0x01` | `OP_INIT_MOD` | Sets up the virtual memory space and execution registers for a module. |
| `0xFF` | `OP_HALT` | Terminates execution and zeroizes module-local stack. |

## 2. Identity & Cryptography

| Opcode | Mnemonic | Description |
|:---:|---|---|
| `0x02` | `OP_IDENTITY_GEN` | Triggers a TRNG-backed identity regeneration. Only valid inside RCF Bunker. |
| `0x03` | `OP_SIGN_DATA` | Signs a provided buffer using the device's identity key. |

## 3. Secure Storage (VFS)

| Opcode | Mnemonic | Description |
|:---:|---|---|
| `0x10` | `OP_VFS_STORE` | Writes a payload to secure flash sector (wear-leveled). Operand: `object_id (2 bytes)`. |
| `0x11` | `OP_VFS_FETCH` | Reads a payload from secure flash. Operand: `object_id (2 bytes)`. |

## 4. Hardware Interaction & Reflex

| Opcode | Mnemonic | Description |
|:---:|---|---|
| `0x20` | `OP_BUS_PUB` | Publishes a telemetry event to the internal system bus. |
| `0xF0` | `OP_REFLEX_ACTION` | **Emergency:** Immediate vault zeroization and system halt. |
| `0xFD` | `OP_LUME_VOICE` | Binary signal pulse (LED/Buzzer pattern) for status feedback. |

## 5. Security Constraints
- **Isolation:** A-VM opcodes cannot access peripheral memory outside their mapped stack/heap.
- **Limit:** Execution is hard-capped at 1,000,000 instructions per call to prevent DoS attacks.

---
*© 2026 Aladdin Aliyev · RCF-PL v1.3*
