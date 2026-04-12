/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Build-time Configuration and Protection Parameters.
 */

#ifndef RCF_CONFIG_H
#define RCF_CONFIG_H

#include <stdint.h>

// Device Identity
#define RCF_DEVICE_NAME         "LUME"
#define RCF_DEVICE_VERSION      "1.3.0"
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
#define RCF_TIMECHAIN_OFFSET    0x0000            // Offset for RCF_Timechain_Entry (128b)
#define RCF_GENESIS_OFFSET      0x0200            // Offset for Genesis Block (128b)
#define RCF_AUDIT_TAIL_OFFSET   0x0400            // 1KB offset for audit logs

// Cryptographic Parameters
#define RCF_CURVE25519_KEY_SIZE     32
#define RCF_ED25519_SIG_SIZE        64
#define RCF_PQC_SIG_SIZE            2420        // Dilithium2
#define RCF_PQC_PUBKEY_SIZE         1312        // Dilithium2
#define RCF_SHA256_HASH_SIZE        32
#define RCF_HMAC_SIZE               32
#define RCF_SESSION_ID_SIZE         16

// Timechain Parameters (v1.3 standards)
#define RCF_MAX_TIME_DRIFT_PPM      500         // ±500 ppm normal
#define RCF_MAX_TIME_DRIFT_FATAL    5000        // >5000 ppm trigger Pill-Off

// [RCF v1.3] Hardware Anchor (Mil-Spec Option A: Encrypted Flash + OTP Key)
#define RCF_MEK_OTP_ADDR            0x1FFF7A00  // 32-byte Master Encryption Key in OTP
#define RCF_MPK_FLASH_ADDR          0x08004000  // Encrypted Dilithium2 PK (1312B) in Flash
#define RCF_MPK_LOCK_ADDR           0x1FFF7A10  // Lock Byte for OTP Block 15
#define RCF_MPK_LOCK_BIT            0x80        // Bit 7 of Lock Byte



// [RCF v1.3] Reflex Thresholds
#define RCF_AVM_MAX_INSTRUCTIONS    1000000U    // 1M instructions per module
#define RCF_AVM_MAX_STACK_DEPTH     16U
#define RCF_REFLEX_HW_THRESHOLD_MV  2800        // 2.8V anomaly trigger

// Protocol Parameters
#define RCF_USB_VID                 0x1209      // pid.codes
#define RCF_USB_PID                 0x7D01      // RCF-Lume v1.3
#define RCF_PROTOCOL_VERSION        0x00010003  // v1.3.0


// Debug (REMOVE IN PRODUCTION)
#ifdef RCF_DEBUG
    #define RCF_LOG_LEVEL           3
    #define RCF_ASSERT_ENABLED      1
#else
    #define RCF_LOG_LEVEL           0
    #define RCF_ASSERT_ENABLED      0
#endif

#endif // RCF_CONFIG_H