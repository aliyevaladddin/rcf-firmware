<!-- NOTICE: This file is protected under RCF-PL v1.2.8 -->
# API Reference — RCF Firmware Modules

Detailed documentation for the core functional modules of the RCF-protected firmware.

## 🗝️ RCF Vault (`rcf-vault.h/c`)
Manages secure storage of license keys and session identities.

### `vault_store_key(uint8_t* key)`
- **Visibility**: [RCF:RESTRICTED]
- **Description**: Stores a 256-bit RCF-PL license key in secure flash.
- **Enforcement**: Requires hardware-backed zeroization support.

## 🛰️ RCF Protocol (`rcf-protocol.h/c`)
Handles the encrypted communication layer over USB.

### `protocol_establish_session()`
- **Visibility**: [RCF:PROTECTED]
- **Description**: Performs a Curve25519-ECDH handshake with the host to derive session keys.
- **Markers**: `@RCF-START[M-SYNC-ESTABLISH]`

### `protocol_send_data(data, len, marker)`
- **Visibility**: [RCF:PROTECTED]
- **Description**: Encrypts and sends data based on the RCF marker provided.

## 🛡️ RCF License (`rcf-license.h/c`)
Validates tier-based feature access.

### `license_check_feature(feature_id)`
- **Visibility**: [RCF:RESTRICTED]
- **Description**: Checks if the stored RCF-PL license key grants access to specific proprietary features (e.g., Enterprise Sync).

---
© 2026 Aladdin Aliyev. All rights reserved.
