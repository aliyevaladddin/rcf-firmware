/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Audit Logging & Security Event Definitions.
 */

#ifndef RCF_AUDIT_H
#define RCF_AUDIT_H

#include <stdint.h>

/* ─── Event Categories ───────────────────────────────────────────────────── */

typedef enum {
    /* VM Events */
    EVENT_VM_VERIFY_START     = 0x10,
    EVENT_VM_VERIFY_OK        = 0x11,
    EVENT_VM_SIG_FAIL         = 0x12,
    EVENT_VM_INVALID_MODULE   = 0x13,
    EVENT_VM_OP_INIT          = 0x14,
    EVENT_VM_IDENTITY_REGEN   = 0x15,
    EVENT_VM_BUS_PUB          = 0x16,
    EVENT_VM_EXT_MOUNT        = 0x17,
    EVENT_VM_REFLEX           = 0x18,
    EVENT_VM_HALT             = 0x19,
    EVENT_VM_UNKNOWN_OP       = 0x1A,
    EVENT_VM_EMERGENCY_RESET  = 0x1B,
    EVENT_VM_MISALIGNED       = 0x1C,

    /* Security & Tamper */

    EVENT_TAMPER_VAULT        = 0x20,
    EVENT_TAMPER_TIME         = 0x21,
    EVENT_TIME_ROLLBACK       = 0x22,
    EVENT_CLOCK_ANOMALY       = 0x23,
    EVENT_BIOMETRIC_FAIL      = 0x24,
    EVENT_INVALID_AUTH        = 0x25,

    /* Session & Protocol */
    EVENT_SESSION_START       = 0x30,
    EVENT_SESSION_END         = 0x31,
    EVENT_CRYPTO_FAIL         = 0x32,
} RCF_Audit_Event;

/**
 * Log a structured security event.
 * In production, this writes to the Secure Audit Tail in Backup SRAM or FLASH.
 */
void rcf_audit_log(RCF_Audit_Event event, uint32_t data);

#endif /* RCF_AUDIT_H */
