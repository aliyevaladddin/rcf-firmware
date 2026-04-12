# RCF-SPEC — Restricted Correlation Framework Specification

**Version:** 1.3 — "The Integrity Release"  
**Status:** Active  
**Author:** Aladdin Aliyev · Sovereign Code Initiative  
**Supersedes:** RCF-SPEC v1.2.8  

---

## Table of Contents

1. [Overview](#1-overview)
2. [Core Principles](#2-core-principles)
3. [Protection Levels](#3-protection-levels)
4. [Secure Boot Flow](#4-secure-boot-flow)
5. [Session Protocol](#5-session-protocol)
6. [Aurora Virtual Machine (A-VM)](#6-aurora-virtual-machine-a-vm)
7. [Timechain & Anti-Rollback](#7-timechain--anti-rollback)
8. [RCF Bunker](#8-rcf-bunker)
9. [Cryptographic Primitives](#9-cryptographic-primitives)
10. [Audit & Compliance](#10-audit--compliance)
11. [Module Index](#11-module-index)

---

## 1. Overview

The **Restricted Correlation Framework (RCF)** is a multi-layer security protocol that operates across three domains simultaneously:

| Domain | Component | Purpose |
|---|---|---|
| **Software** | RCF-PL License + rcf-cli | IP protection, marker enforcement |
| **Firmware** | Aurora Access / A-VM | Tamper-resistant code execution |
| **Hardware** | RCF Bunker + Timechain | Root-of-Trust, anti-rollback |

---

## 2. Core Principles

### 2.1 Visibility ≠ Rights
Visibility is granted unconditionally for audit purposes. Usage (replication, AI training) requires explicit authorization.

### 2.2 Layered Defense
RCF enforces protection at four independent layers: Legal, Tooling, Firmware, Hardware.

---

## 3. Protection Levels

| Marker | Hex (wire) | Scope |
|---|:---:|---|
| `[RCF:PUBLIC]` | `0x01` | Architecture, interfaces, public APIs |
| `[RCF:PROTECTED]` | `0x02` | Core methodology, algorithmic logic |
| `[RCF:RESTRICTED]` | `0x03` | Sensitive implementation, key material |

---

## 4. Secure Boot Flow

```mermaid
flowchart TD
    A([Power On / Reset]) --> B[STM32F4 ROM Bootloader]
    B --> C{RCF Vault\nInitialized?}
    
    C -- No --> D[Factory Provisioning\nRequired]
    D --> E[rcf-provision.py\nVault + Genesis injection]
    E --> F[PQC Sign\nrcf-signer Dilithium2]
    F --> G([Provisioned Image])

    C -- Yes --> H[Timechain Init\nBackup SRAM]
    H --> I{Chain\nIntegrity OK?}
    
    I -- Tampered --> J[trigger_pill_off\nPILL_OFF_TAMPER_TIME]
    J --> K([EMERGENCY HALT])
    
    I -- OK --> L[RCF Bunker Init\nIsolated crypto zone]
    L --> M[Load Sentinel MPK\nfrom Vault]
    M --> N[A-VM Ready]
    N --> O([Normal Operation])

    style A fill:#1D9E75,color:#fff
    style K fill:#D85A30,color:#fff
    style G fill:#1D9E75,color:#fff
    style O fill:#1D9E75,color:#fff
    style J fill:#D85A30,color:#fff
```

### 4.1 Boot Stages
1. **ROM:** Integrity validation.
2. **Vault Check:** Provisioning verification via Backup SRAM.
3. **Timechain:** Monotonic time and rollback protection.
4. **Bunker:** Cryptographic isolation.
5. **A-VM:** Signed .acode execution readiness.

---

## 5. Session Protocol

```mermaid
sequenceDiagram
    participant H as Host
    participant D as Device (A-VM)
    participant B as RCF Bunker

    H->>D: RCF_CMD_HELLO
    D->>D: curve25519_keygen
    D->>H: RCF_CMD_CHALLENGE (eph_pub + nonce)
    H->>D: [host_eph_pub(32)] + [host_sig(64)]
    Note over D: ECDH + HKDF-SHA256
    D->>H: RCF_CMD_SESSION (AES-256-GCM)
```

---

## 6. Aurora Virtual Machine (A-VM)

### 6.1 Purpose
The A-VM executes exclusively verified `.acode` modules signed with Dilithium2.

---

## 7. Timechain & Anti-Rollback
A SHA-256 linked list stored in Backup SRAM (VBAT-powered).

---

## 9. Cryptographic Primitives
- **Curve25519**: Session ECDH.
- **AES-256-GCM**: Frame encryption.
- **Dilithium2 (NIST PQC)**: Module signing.
- **SHA-256**: HW accelerated integrity.

---

*© 2026 Aladdin Aliyev · All rights reserved.*  
*Protected under RCF-PL v1.3 — Sovereignty via Restricted Correlation.*
