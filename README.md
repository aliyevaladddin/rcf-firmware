# RCF Firmware — Aurora Access

> **Root-of-Trust for the dOS Ecosystem**

[![RCF Protected](https://img.shields.io/badge/license-RCF--PL%20v1.3-7F77DD?style=flat-square)](../../LICENSE)
[![Status](https://img.shields.io/badge/status-active-1D9E75?style=flat-square)]()
[![Target](https://img.shields.io/badge/target-ARM64%20%7C%20STM32-534AB7?style=flat-square)]()

---

## Overview

**RCF Firmware** is the secure hardware root-of-trust for the **Aurora Access** ecosystem and the **dOS** (Decentralized Operating System). It is designed for high-security node coordination and tamper-resistant execution on embedded hardware.

This project is governed by the **Restricted Correlation Framework (RCF)**. It is **Visible Source** — architectural study is encouraged, but core methodologies are protected under [RCF-PL v1.3](../../LICENSE).

---

## Key Features

| Component | Description |
|---|---|
| **A-VM** | Aurora Virtual Machine — secure execution engine for signed `.acode` modules |
| **PQC Security** | Post-Quantum verification via **Dilithium2** (NIST PQC standard) |
| **Embedded Modules** | Filesystem-independent boot modules (Heartbeat, Identity) as static C arrays |
| **RCF Hardware Protocol v1.2.9** | Standardized peripheral abstraction (Flash, SD/MMC, SPI, UART) |
| **RCF Bunker** | Isolated shielding mode for cryptographic key operations |
| **Factory Provisioning** | Per-device identity and license injection at manufacture time |

---

## Project Structure

```
AuroraAccess/
├── core/
│   ├── src/          — Proprietary firmware logic      [RCF:PROTECTED]
│   └── inc/          — Hardware & protocol headers     [RCF:PUBLIC]
├── modules/          — Binary .acode source modules    [RCF:RESTRICTED]
├── DOCUMENTATION/
│   ├── HARDWARE-PROTOCOL.md   — RCF Hardware Protocol spec
│   ├── SDIO-SPEC.md           — SDIO/SD-MMC specification
│   ├── SPI-SPEC.md            — SPI bus specification
│   └── UART-SPEC.md           — UART specification
└── build/
    └── Makefile      — Build system (STM32F407 reference)

tools/
└── rcf-provision.py  — Factory provisioning tool       [RCF:RESTRICTED]
```

---

## Provisioning

Each device receives a unique cryptographic identity at manufacturing time via `rcf-provision.py`.

### Prerequisites

```bash
pip install cryptography
export RCF_SIGNER_PATH=./bin/rcf-signer   # path to PQC signer binary
```

### Usage

```bash
# Full provisioning + PQC signing
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --uid    DEVICE-ARM64-001 \
  --manifest

# Preview without writing (dry-run)
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --dry-run

# Provision without signing (testing)
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --no-sign
```

### Output

```
[RCF] Loaded binary: build/aurora.bin (131,072 bytes)
[RCF] Starting provisioning — UID: DEVICE-ARM64-001
[RCF] Vault constructed — 234 bytes, version 0x0103
[INFO] Vault marker at offset 0x00001000 — injecting.
[RCF] Calling native signer: ./bin/rcf-signer
[RCF] Manifest written: build/aurora.provisioned.manifest.json

[SUCCESS] Provisioned image: build/aurora.provisioned.bin
  Device Pubkey : A3F1...
  Genesis Hash  : 7B2C...
```

---

## Build Firmware

```bash
cd AuroraAccess/build
make

# Clean build
make clean && make

# With debug symbols
make DEBUG=1
```

---

## Compliance & Audit

This project enforces **100% RCF compliance** using `rcf-cli`.

```bash
# Install rcf-cli
pip install rcf-cli

# Check compliance
rcf-cli protect AuroraAccess/ --dry-run

# Generate audit report
rcf-cli audit AuroraAccess/ --license-key $RCF_LICENSE_KEY

# Verify integrity (CI/CD)
rcf-cli diff AuroraAccess/
```

For CI/CD, add `rcf-guardian` to your workflow:

```yaml
- uses: aliyev/rcf-guardian@v1
  with:
    path: 'AuroraAccess/'
    mode: 'diff'
```

---

## Security Model

```
┌─────────────────────────────────────┐
│           Hardware Node             │
│  ┌───────────────────────────────┐  │
│  │         RCF Bunker            │  │  ← Isolated crypto zone
│  │   (Dilithium2 / X25519 keys)  │  │
│  └───────────┬───────────────────┘  │
│              │                      │
│  ┌───────────▼───────────────────┐  │
│  │      A-VM (Aurora VM)         │  │  ← Only signed .acode runs
│  │   [RCF:RESTRICTED] core       │  │
│  └───────────┬───────────────────┘  │
│              │                      │
│  ┌───────────▼───────────────────┐  │
│  │   RCF Hardware Protocol       │  │  ← Unified peripheral access
│  │   SD / Flash / SPI / UART     │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
         ▲
         │ Factory Provisioning
   rcf-provision.py
   (Vault + Genesis + License)
```

---

## Documentation

- [RCF Hardware Protocol](DOCUMENTATION/HARDWARE-PROTOCOL.md)
- [SDIO Specification](DOCUMENTATION/SDIO-SPEC.md)
- [SPI Specification](DOCUMENTATION/SPI-SPEC.md)
- [UART Specification](DOCUMENTATION/UART-SPEC.md)
- [RCF-PL v1.3 License](../../LICENSE)
- [RCF Whitepaper](../../WHITE_PAPER_v1.3.md)

---

*© 2026 Aladdin Aliyev. All rights reserved.*  
*Protected under RCF-PL v1.3 — Sovereignty via Restricted Correlation.*
