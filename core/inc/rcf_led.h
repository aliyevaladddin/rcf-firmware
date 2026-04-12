/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * LED Indicator Control Definitions.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_LED_H
#define RCF_LED_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LED_PATTERN_IDLE        = 0,
    LED_PATTERN_BOOTING     = 1,
    LED_PATTERN_AUTH_WAIT   = 2,
    LED_PATTERN_SUCCESS     = 3,
    LED_PATTERN_ERROR       = 4,
    LED_PATTERN_TAMPER      = 5,
    LED_PATTERN_PILL_OFF    = 6
} LED_Pattern;

void led_init(void);
void led_set_pattern(LED_Pattern pattern);

/**
 * [RCF v1.3] Emergency Blackout
 * Immediately kills all UI feedback to prevent information leakage during zeroization.
 */
void led_set_void_black(void);

#endif /* RCF_LED_H */
