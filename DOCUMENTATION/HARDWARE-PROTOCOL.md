# RCF Hardware Protocol Specification (v1.3)

[ [RCF:PROTECTED] ]

This document describes the binary wire protocol used for communication between a Host (dOS) and an Aurora Access node.

## 1. Physical Layer
- **Interface:** USB CDC (Virtual COM Port)
- **Baud Rate:** Not applicable (USB High Speed)
- **Framing:** RCF Packet Headers

## 2. Framing Structure
Every packet starts with a 32-byte header:

```c
typedef struct __attribute__((packed)) {
    uint8_t  magic[4];      // "RCF\0"
    uint16_t version;       // 0x0103 (v1.3)
    uint8_t  command;       // RCF_CMD_*
    uint16_t payload_len;   // Length of data following header
    uint8_t  marker;        // 0x01:PUBLIC, 0x02:PROTECTED, 0x03:RESTRICTED
    uint16_t session_id;    // Assigned during handshake
    uint16_t reserved;      // 0x0000
    uint16_t flags;         // RCF_FLAG_*
    uint8_t  auth_tag[16];  // AES-GCM tag or HMAC (depending on mode)
} RCF_Packet_Header;
```

## 3. Session Handshake (ECDH)

Communication is protected via a session key established through an Elliptic Curve Diffie-Hellman (ECDH) exchange.

1. **Discovery:** Host sends `RCF_CMD_HELLO`.
2. **Challenge:** Device generates ephemeral Curve25519 keypair and sends `RCF_CMD_CHALLENGE` with its public key and a 16-byte nonce.
3. **Response:** Host generates its own keypair, computes shared secret, and sends its public key + signature.
4. **Activation:** Both parties derive `enc_key` and `mac_key` using HKDF-SHA256.

## 4. Encryption Policy

- **PUBLIC:** Plaintext frames, authenticated header.
- **PROTECTED / RESTRICTED:** Payload encrypted with AES-256-GCM.

---
*© 2026 Aladdin Aliyev · RCF-PL v1.3*
