/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * RCF-PL v1.2.7 — Restricted Correlation Framework
 * Secure Timechain & Anti-Rollback Protection Definitions.
 */

#ifndef RCF_TIMECHAIN_H
#define RCF_TIMECHAIN_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint64_t timestamp_unix;         // Seconds since epoch
    uint32_t monotonic_counter;        // Never decreases
    uint16_t subsecond_ticks;          // LSI ticks (1/32768 s)
    uint8_t  entropy_pool[16];       // TRNG mix-in
    
    // Blockchain links
    uint8_t  prev_hash[32];            // SHA-256 of previous entry
    uint8_t  self_hash[32];            // SHA-256 of this entry
    
    // Metadata
    uint32_t power_cycle_count;
    int16_t  temperature_celsius;      // Internal sensor
    uint16_t voltage_mv;               // VBAT voltage
} RCF_Timechain_Entry;

// Lifecycle
bool timechain_init(void);
bool timechain_is_valid(void);

// Time operations
bool timechain_get_timestamp(uint64_t* out_timestamp);
bool timechain_update(void);
bool timechain_verify_monotonicity(uint64_t candidate);

// Genesis block (factory provisioning)
bool timechain_create_genesis(uint64_t factory_timestamp);

// Tamper detection
bool timechain_detect_rollback(uint64_t new_time);
bool timechain_detect_clock_anomaly(void);

#endif // RCF_TIMECHAIN_H