/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Pill-Off Reasons — RC2 Extended.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_PILLOFF_H
#define RCF_PILLOFF_H

#include <stdint.h>

typedef enum {
    /* Hardware/Physical */
    PILL_OFF_TAMPER_COVER = 0x01,      /* Physical intrusion */
    PILL_OFF_TAMPER_TIME = 0x02,       /* Time rollback detected */
    PILL_OFF_TAMPER_CLOCK = 0x03,      /* Clock manipulation */
    PILL_OFF_TAMPER_CLONE = 0x04,      /* Device UID mismatch */
    
    /* Software/Security */
    PILL_OFF_INVALID_AUTH = 0x10,      /* Biometric failures exceeded */
    PILL_OFF_INVALID_LICENSE = 0x11,   /* License validation failed */
    PILL_OFF_TAMPER_CODE = 0x12,       /* Code integrity failure */
    PILL_OFF_VAULT_CORRUPTION = 0x13,  /* Vault init failure after retries */
    PILL_OFF_TRNG_FAULT = 0x14,        /* TRNG health check failure */
    PILL_OFF_WATCHDOG = 0x15,          /* System hang detected */
    PILL_OFF_ACODE = 0x16,             /* A-VM execution violation */
    
    /* Async/Immediate */
    PILL_OFF_EXTERNAL = 0xEE,          /* Triggered via bridge from dOS */
    PILL_OFF_ASYNC = 0xF0,             /* Asynchronous trigger */
    PILL_OFF_IMMEDIATE = 0xFF          /* Emergency shutdown */
} PillOff_Reason;

/* Function prototypes */
void trigger_pill_off(PillOff_Reason reason);
void trigger_async_pill_off(void);
void pill_off_immediate(void);

#endif /* RCF_PILLOFF_H */
