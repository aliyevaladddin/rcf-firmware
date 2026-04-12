/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * Security Audit Logging.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_AUDIT_H
#define RCF_AUDIT_H

#include <stdint.h>
#include <stdio.h>
#include "rcf_config.h"

/* Event types */
typedef enum {
    EVENT_PILL_OFF_TRIGGERED = 0x01,
    EVENT_PILL_OFF_IMMEDIATE = 0x02,
    EVENT_PULSE_INIT = 0x10,
    EVENT_PULSE_LIVENESS = 0x11,
    EVENT_VM_VERIFY_START = 0x20,
    EVENT_VM_VERIFY_OK = 0x22,
    EVENT_VM_SIG_FAIL = 0x21,
    EVENT_TAMPER_VAULT = 0x30,
    EVENT_TIME_ROLLBACK = 0x31,
    EVENT_CLOCK_ANOMALY = 0x32,
} AuditEvent;

/* Log event with optional data */
void rcf_audit_log(AuditEvent event, uint32_t data);

#ifdef RCF_VM_CI_MODE
    /* CI: printf-based logging */
    #define RCF_AUDIT_LOG(event, data) printf("[AUDIT] %02X: %08X\n", (unsigned int)event, (unsigned int)data)
#else
    /* Production: secure flash storage */
    #define RCF_AUDIT_LOG(event, data) rcf_audit_log(event, data)
#endif

#endif /* RCF_AUDIT_H */
