/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * Biometric Pulse Sensor — Liveness Detection.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_PULSE_H
#define RCF_PULSE_H

#include <stdint.h>
#include <stdbool.h>
#include "rcf_config.h"

/* Initialize pulse sensor hardware */
bool pulse_init(void);

/* Check for finger presence */
bool pulse_check_activation(void);

/* Validate liveness (anti-spoofing) */
bool pulse_validate_liveness(void);

/* Get failed authentication attempts counter */
uint8_t get_failed_attempts(void);

/* Reset authentication state */
void pulse_reset_auth(void);

#endif /* RCF_PULSE_H */
