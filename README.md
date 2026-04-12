# RCF-Firmware

> **Root-of-Trust for the dOS Ecosystem · Aurora Access · STM32F407**

[![Build](https://github.com/aliyevaladddin/rcf-firmware/actions/workflows/rc2-hardened.yml/badge.svg)](https://github.com/aliyevaladddin/rcf-firmware/action)
[![License: RCF-PL v1.3](https://img.shields.io/badge/license-RCF--PL%20v1.3-7F77DD?style=flat-square)](LICENSE)
[![Status](https://img.shields.io/badge/status-RC2--LS-1D9E75?style=flat-square)]()
[![Target](https://img.shields.io/badge/target-STM32F407-534AB7?style=flat-square)]()
[![QEMU CI](https://img.shields.io/badge/QEMU%20CI-passing-1D9E75?style=flat-square)]()

---

**RCF-Firmware** is a hardened firmware implementation of the **Restricted Correlation Framework**, designed for **Aurora Access** nodes. Version RC2-LS (Life Support) delivers a full bare-metal security stack on the STM32F407: from hardware Root-of-Trust to verified bytecode execution.

This project is governed by **RCF-PL v1.3** — Visible Source, Protected Logic. The architecture is open for audit. The methodology is protected.

---

## RC2-LS Status

| Component | Status |
|---|---|
| A-VM (Aurora Virtual Machine) | ✅ Active |
| Timechain Anti-Rollback | ✅ Active |
| RCF Bunker (Crypto Isolation) | ✅ Active |
| Factory Provisioning | ✅ Active |
| QEMU CI Verification | ✅ 100% Green |
| PQC Dilithium2 | ✅ Integrated |

---

## Key Components

| Component | Description |
|---|---|
| **A-VM** | Aurora Virtual Machine — executes only cryptographically signed `.acode` modules |
| **PQC Security** | Post-Quantum verification via **Dilithium2** (NIST PQC Level 2) |
| **RCF Bunker** | Isolated execution mode for cryptographic key operations |
| **Timechain** | SHA-256 linked chain in Backup SRAM — rollback and physical tamper protection |
| **Hardware Protocol v1.2.9** | Unified peripheral abstraction: Flash, SD/MMC, SPI, UART |
| **Factory Provisioning** | Unique cryptographic device identity injected at manufacture time |

---

## Project Structure

```
rcf-firmware/
├── AuroraAccess/
│   ├── core/
│   │   ├── src/              [RCF:PROTECTED]  — Firmware logic
│   │   └── inc/              [RCF:PUBLIC]     — Headers and protocol definitions
│   ├── modules/              [RCF:RESTRICTED] — Binary .acode modules
│   └── DOCUMENTATION/
│       ├── RCF-SPECIFICATION.md  — Master protocol specification
│       ├── HARDWARE-PROTOCOL.md  — Host to Device data exchange
│       ├── AURORA-VM-OPCODES.md  — A-VM opcode reference
│       ├── SECURITY-MODEL.md     — Protection levels and threat model
│       ├── SDIO-SPEC.md
│       ├── SPI-SPEC.md
│       └── UART-SPEC.md
├── tools/
│   └── rcf-provision.py     [RCF:RESTRICTED] — Factory provisioning tool
├── tests/
│   └── qemu_verify.py        — QEMU CI verification
└── build/
    └── Makefile              — STM32F407 build system
```

---

## Build

```bash
# RC2 production build
make clean && make RC2
# Output: .build/rcf-lume-rc2.bin

# Debug build
make DEBUG=1

# QEMU CI verification
python3 tests/qemu_verify.py .build/rcf-lume-rc2.elf
# [RCF-TEST] CI Verification PASSED.
```

---

## Factory Provisioning

Every device receives a unique cryptographic identity at manufacturing time via `rcf-provision.py`.

**Prerequisites:**

```bash
pip install cryptography
export RCF_SIGNER_PATH=./bin/rcf-signer
```

**Usage:**

```bash
# Full provisioning + PQC signing
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --uid    DEVICE-ARM64-001 \
  --manifest

# Preview without writing
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --dry-run

# Provision without signing (testing only)
python tools/rcf-provision.py \
  --device build/aurora.bin \
  --output build/aurora.provisioned.bin \
  --no-sign
```

**Output:**

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

## Compliance & Audit

```bash
pip install rcf-cli

rcf-cli protect AuroraAccess/ --dry-run            # preview marker insertion
rcf-cli audit   AuroraAccess/ --license-key $RCF_LICENSE_KEY
rcf-cli diff    AuroraAccess/                      # CI integrity check
```

**GitHub Action (rcf-guardian):**

```yaml
- uses: aliyev/rcf-guardian@v1
  with:
    path: 'AuroraAccess/'
    mode: 'diff'
```

---

## Security Model

```
┌──────────────────────────────────────────┐
│              Hardware Node               │
│  ┌────────────────────────────────────┐  │
│  │           RCF Bunker               │  │  <- Isolated crypto zone
│  │   Dilithium2 / X25519 / AES-GCM    │  │     Keys never leave this context
│  └──────────────┬─────────────────────┘  │
│                 │                        │
│  ┌──────────────▼─────────────────────┐  │
│  │       A-VM (Aurora VM)             │  │  <- Only signed .acode executes
│  │       [RCF:RESTRICTED]             │  │     Dilithium2 verified
│  └──────────────┬─────────────────────┘  │
│                 │                        │
│  ┌──────────────▼─────────────────────┐  │
│  │      RCF Hardware Protocol         │  │  <- SD / Flash / SPI / UART
│  │             v1.2.9                 │  │     Unified peripheral access
│  └────────────────────────────────────┘  │
│                 ▲                        │
│  ┌──────────────┴─────────────────────┐  │
│  │     Timechain (Backup SRAM)        │  │  <- Anti-rollback
│  │     SHA-256 chain · VBAT-backed    │  │     Physical tamper detection
│  └────────────────────────────────────┘  │
└──────────────────────────────────────────┘
              ▲
              │ Factory Provisioning
        rcf-provision.py
        Vault + Genesis + License
```

---

## Documentation

| Document | Description |
|---|---|
| [RCF-SPECIFICATION.md](DOCUMENTATION/RCF-SPECIFICATION.md) | Master protocol specification v1.3 |
| [HARDWARE-PROTOCOL.md](DOCUMENTATION/HARDWARE-PROTOCOL.md) | Host to Device data exchange |
| [AURORA-VM-OPCODES.md](DOCUMENTATION/AURORA-VM-OPCODES.md) | A-VM opcode reference |
| [SECURITY-MODEL.md](DOCUMENTATION/SECURITY-MODEL.md) | Protection levels and threat model |
| [SDIO-SPEC.md](DOCUMENTATION/SDIO-SPEC.md) | SDIO / SD-MMC specification |
| [SPI-SPEC.md](DOCUMENTATION/SPI-SPEC.md) | SPI bus specification |
| [UART-SPEC.md](DOCUMENTATION/UART-SPEC.md) | UART specification |
| [RCF-PL v1.3 License](LICENSE) | Full license text |
| [RCF Whitepaper](../../WHITE_PAPER_v1.3.md) | Technical and legal rationale |

---

*© 2026 Aladdin Aliyev. All rights reserved.*
*Protected under RCF-PL v1.3 — Sovereignty via Restricted Correlation.*