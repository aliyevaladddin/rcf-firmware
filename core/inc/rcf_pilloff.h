/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Emergency Zeroization (Pill-Off) Definitions.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_PILLOFF_H
#define RCF_PILLOFF_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PILL_OFF_TAMPER        = 0x01,
    PILL_OFF_VAULT         = 0x02,
    PILL_OFF_STACK         = 0x03,
    PILL_OFF_TRNG          = 0x04,
    PILL_OFF_ACODE         = 0x05,
    PILL_OFF_ASYNC         = 0x06
} PillOff_Reason;

void trigger_pill_off(PillOff_Reason reason);
void trigger_async_pill_off(void);
void pill_off_immediate(void);

#endif /* RCF_PILLOFF_H */
