// [RCF:PUBLIC] — Build-time configuration
#ifndef RCF_CONFIG_H
#define RCF_CONFIG_H

#include <stdint.h>

// Device Identity
#define RCF_DEVICE_NAME         "LUME"
#define RCF_DEVICE_VERSION      "1.0.0"
#define RCF_DEVICE_MAGIC        0x4C554D45  // "LUME"

// Hardware Specification (STM32F407VG)
#define RCF_FLASH_SIZE          (1024 * 1024)   // 1MB
#define RCF_SRAM_SIZE           (128 * 1024)    // 128KB
#define RCF_BACKUP_SRAM_SIZE    (4 * 1024)      // 4KB on VBAT

// Memory Layout
#define RCF_VAULT_SECTOR        FLASH_SECTOR_11   // Last 128KB sector
#define RCF_VAULT_ADDR          0x080E0000
#define RCF_VAULT_SIZE          0x00020000        // 128KB

#define RCF_BACKUP_SRAM_ADDR    0x40024000
#define RCF_TIMECHAIN_OFFSET    0x0000            // 256 bytes
#define RCF_AUDIT_TAIL_OFFSET   0x0100            // 256 bytes
#define RCF_GENESIS_OFFSET      0x0200            // 256 bytes

// Cryptographic Parameters
#define RCF_CURVE25519_KEY_SIZE     32
#define RCF_ED25519_SIG_SIZE        64
#define RCF_SHA256_HASH_SIZE        32
#define RCF_HMAC_SIZE               32
#define RCF_SESSION_ID_SIZE         16

// Timechain Parameters
#define RCF_TIMECHAIN_INTERVAL_MS   1000        // 1 second updates
#define RCF_MAX_TIME_DRIFT_PPM      5000        // 0.5% max drift
#define RCF_ROLLBACK_THRESHOLD_MS   1000        // 1 second grace

// License Parameters
#define RCF_LICENSE_FEATURES_MAX    8           // Bitmask: 8 features
#define RCF_LICENSE_HMAC_SIZE       32

// Pill-Off Parameters
#define RCF_MAX_AUTH_ATTEMPTS       10
#define RCF_AUTH_LOCKOUT_MS         300000      // 5 minutes
#define RCF_SHREDDER_CHARGE_MS      10000       // 10s supercap charge
#define RCF_SHREDDER_DISCHARGE_MS   100         // 100ms pulse

// LED Parameters
#define RCF_LED_PWM_FREQ            1000        // 1kHz PWM
#define RCF_LED_NEURAL_CYAN_R       0
#define RCF_LED_NEURAL_CYAN_G       128
#define RCF_LED_NEURAL_CYAN_B       255

// Protocol Parameters
#define RCF_USB_VID                 0x1209      // pid.codes
#define RCF_USB_PID                 0x7D01      // RCF-Lume v1
#define RCF_PROTOCOL_VERSION        0x00010000  // 1.0.0

// Debug (REMOVE IN PRODUCTION)
#ifdef RCF_DEBUG
    #define RCF_LOG_LEVEL           3
    #define RCF_ASSERT_ENABLED      1
#else
    #define RCF_LOG_LEVEL           0
    #define RCF_ASSERT_ENABLED      0
#endif

#endif // RCF_CONFIG_H