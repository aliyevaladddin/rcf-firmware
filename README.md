# RCF Firmware — Aurora Access Ecosystem

**Version:** v1.2.9 (Alpha)  
**Protocol:** RCF-PL v1.2.8 (Restricted Correlation Framework)  
**Registry:** [rcf.aliyev.site](https://rcf.aliyev.site)

---

## 🛡️ Overview

**RCF Firmware** is the secure hardware root-of-trust for the **Aurora Access** ecosystem and the dOS (Decentralized Operating System). It is specifically designed for high-security node coordination and tamper-resistant execution.

This project is governed by the **Restricted Correlation Framework (RCF)**. It is **Visible Source**, meaning architectural study is encouraged, but core methodologies are protected under the RCF-PL license.

## 🚀 Key Features

- **A-VM (Aurora Virtual Machine)**: Secure execution engine for signed `.acode` modules.
- **PQC Security**: Post-Quantum Cryptographic verification using **Dilithium2**.
- **Embedded Modules**: Filesystem-independent boot modules (Heartbeat, Identity) embedded directly as static C arrays.
- **EHA Protocol (v1.2.9)**: Standardized **External Hardware Abstraction** for peripheral connectivity (Flash cards, SD/MMC).
- **RCF Bunker**: Isolated shielding mode for sensitive cryptographic operations.

## 🧩 Project Structure

- `AuroraAccess/core/src/` — Proprietary firmware logic.
- `AuroraAccess/core/inc/` — Hardware & Protocol headers.
- `AuroraAccess/modules/` — Binary `.acode` source modules.
- `AuroraAccess/DOCUMENTATION/` — Technical specs (EHA, Security, API).
- `build/Makefile` — Standardized build system (STM32F407 Reference).

## 🛠️ Compliance & Audit

We enforce a strict **100% compliance** policy using the `rcf-cli` suite.

### Check Compliance Locally
```bash
pip install rcf-cli  # or install from rcf-protocol repo
rcf-cli check AuroraAccess/
```

### Build Firmware
```bash
cd build
make
```

---

© 2026 Aladdin Aliyev. All rights reserved.
