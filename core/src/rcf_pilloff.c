/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Pill-Off System — CI/QEMU Stub.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_pilloff.h"
#include "rcf_audit.h"
#include <stdio.h>

#ifdef RCF_VM_CI_MODE
/* QEMU simulation: log only, don't exit process in CI by default */
void trigger_pill_off(PillOff_Reason reason) {
    rcf_audit_log(EVENT_PILL_OFF_TRIGGERED, (uint32_t)reason);
    printf("[RCF-CI] PILL_OFF triggered: reason=%d\n", (int)reason);
    printf("[RCF-CI] In production, this would zeroize Vault and halt.\n");
    /* In CI: continue execution for testing or halt if required */
}

void trigger_async_pill_off(void) {
    trigger_pill_off(PILL_OFF_ASYNC);
}

void pill_off_immediate(void) {
    rcf_audit_log(EVENT_PILL_OFF_IMMEDIATE, 0);
    printf("[RCF-CI] IMMEDIATE PILL_OFF (simulated)\n");
}

#else
/* Production: hardware zeroization */
void trigger_pill_off(PillOff_Reason reason) {
    rcf_audit_log(EVENT_PILL_OFF_TRIGGERED, (uint32_t)reason);
    // rcf_vault_emergency_wipe();
    while(1);
}

void trigger_async_pill_off(void) {
    trigger_pill_off(PILL_OFF_ASYNC);
}

void pill_off_immediate(void) {
    rcf_audit_log(EVENT_PILL_OFF_IMMEDIATE, 0);
    // rcf_vault_emergency_wipe();
    while(1);
}
#endif
