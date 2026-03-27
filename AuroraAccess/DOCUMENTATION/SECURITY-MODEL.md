# Security Model — RCF Firmware

This document outlines the protection boundaries and trust assumptions for the RCF-protected firmware.

## 1. Protection Boundaries
The firmware distinguishes between **Execution Integrity** and **Logic Confidentiality**.

| Zone | Visibility | Protection |
|:---|:---|:---|
| **Public** | Open | Standard Git tracking |
| **Protected** | Auditable | RCF Markers + PL License |
| **Restricted** | Limited | RCF Markers + Cryptographic Vault |

## 2. Threat Model

### T1: Automated Logic Extraction
- **Mitigation**: RCF Markers (`@RCF-START`) trigger `rcf-cli` enforcement, preventing unauthorized reuse of logic in automated systems.
- **Enforcement**: Legally backed by RCF-PL v1.2.7.

### T2: Unauthorized Protocol Replication
- **Mitigation**: Protocol handshake requires a valid RCF-PL license key stored in the `RCF Vault`.
- **Enforcement**: Functional dependency on restricted cryptographic features.

## 3. Vault Integrity
The `rcf-vault` implementation utilizes STM32 hardware crypto features to ensure that session keys jamais quit the secure boundary in plaintext.

---
© 2026 Aladdin Aliyev. All rights reserved.
