<!-- NOTICE: This file is protected under RCF-PL v1.2.8 -->
# RCF-LUME-FIRMWARE ROADMAP
├── Core/
│   ├── Inc/
│   │   ├── main.h                    # [RCF:PUBLIC] Entry point
│   │   ├── rcf_config.h              # [RCF:PUBLIC] Build configuration
│   │   ├── rcf_hal.h                 # [RCF:PUBLIC] HAL abstraction
│   │   ├── rcf_vault.h               # [RCF:RESTRICTED] Key management
│   │   ├── rcf_timechain.h           # [RCF:RESTRICTED] Secure time
│   │   ├── rcf_license.h             # [RCF:RESTRICTED] License validation
│   │   ├── rcf_crypto.h              # [RCF:RESTRICTED] Cryptographic engine
│   │   ├── rcf_protocol.h            # [RCF:PROTECTED] RCF USB protocol
│   │   ├── rcf_audit.h               # [RCF:PROTECTED] Audit logging
│   │   ├── rcf_pulse.h               # [RCF:PROTECTED] Biometric sensor
│   │   ├── rcf_pilloff.h             # [RCF:RESTRICTED] Zeroization
│   │   └── rcf_led.h                 # [RCF:PUBLIC] LED control
│   └── Src/
│       ├── main.c                    # [RCF:PUBLIC]
│       ├── system_stm32f4xx.c        # [RCF:PUBLIC] CMSIS
│       ├── rcf_vault.c               # [RCF:RESTRICTED]
│       ├── rcf_timechain.c           # [RCF:RESTRICTED]
│       ├── rcf_license.c             # [RCF:RESTRICTED]
│       ├── rcf_crypto.c              # [RCF:RESTRICTED]
│       ├── rcf_protocol.c            # [RCF:PROTECTED]
│       ├── rcf_audit.c               # [RCF:PROTECTED]
│       ├── rcf_pulse.c               # [RCF:PROTECTED]
│       ├── rcf_pilloff.c             # [RCF:RESTRICTED]
│       └── rcf_led.c                 # [RCF:PUBLIC]
├── Middleware/
│   ├── USB_Device/
│   │   └── CDC/                      # [RCF:PUBLIC] USB CDC class
│   └── Crypto/
│       ├── curve25519/               # [RCF:RESTRICTED] Ref10 implementation
│       ├── sha256_stm32f4/           # [RCF:RESTRICTED] HW-accelerated
│       └── ed25519/                  # [RCF:RESTRICTED] Ref10
├── Drivers/
│   └── STM32F4xx_HAL_Driver/         # [RCF:PUBLIC] ST HAL
└── Build/
    ├── Makefile                      # [RCF:PUBLIC]
    └── rcf_provision.py              # [RCF:RESTRICTED] Factory tool