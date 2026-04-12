/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Secure Timechain & Anti-Rollback Protection Definitions.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#ifndef RCF_TIMECHAIN_H
#define RCF_TIMECHAIN_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Constants ──────────────────────────────────────────────────────────── */

#define RCF_TIMECHAIN_HASH_LEN      32U
#define RCF_TIMECHAIN_ENTROPY_LEN   16U
#define RCF_TIMECHAIN_ENTRY_SIZE    128U   /* Must match backup SRAM layout  */

/* LSI nominal frequency and max allowed drift */
#define RCF_LSI_NOMINAL_HZ          32000U
#define RCF_MAX_TIME_DRIFT_PPM      500U   /* ±500 ppm normal operation      */
#define RCF_MAX_TIME_DRIFT_FATAL    5000U  /* >5000 ppm → pill-off           */

/* Anti-rollback: max tolerated backwards delta before alarm */
#define RCF_MAX_BACKWARD_DELTA_SEC  5U

/* ─── Timechain entry ────────────────────────────────────────────────────── */

/*
 * Layout in Backup SRAM (128 bytes, little-endian):
 *
 *   Offset  Size  Field
 *   0       8     timestamp_unix       seconds since Unix epoch
 *   8       4     monotonic_counter    strictly increasing, never wraps
 *   12      2     subsecond_ticks      RTC sub-second register (SSR)
 *   14      2     temperature_celsius  STM32 internal sensor (signed)
 *   16      2     voltage_mv           VBAT voltage in millivolts
 *   18      2     power_cycle_count    (truncated; full 32-bit below)
 *   20      4     power_cycle_count    full 32-bit value
 *   24      4     lsi_frequency_hz     last measured LSI frequency
 *   28      4     reserved             must be zero
 *   32      16    entropy_pool         TRNG mix-in for next hash
 *   48      32    prev_hash            SHA-256 of previous entry
 *   80      32    self_hash            SHA-256 of bytes [0:80]
 *   112     16    reserved2            future use, must be zero
 *   --- Total: 128 bytes ---
 */
typedef struct __attribute__((packed)) {
    uint64_t timestamp_unix;                        /* Seconds since epoch   */
    uint32_t monotonic_counter;                     /* Strictly increasing   */
    uint16_t subsecond_ticks;                       /* RTC SSR value         */
    int16_t  temperature_celsius;                   /* Internal sensor       */
    uint16_t voltage_mv;                            /* VBAT in millivolts    */
    uint16_t power_cycle_count_lo;                  /* Low 16 bits           */
    uint32_t power_cycle_count;                     /* Full 32-bit counter   */
    uint32_t lsi_frequency_hz;                      /* Last LSI measurement  */
    uint32_t reserved;                              /* Must be zero          */
    uint8_t  entropy_pool[RCF_TIMECHAIN_ENTROPY_LEN]; /* TRNG mix-in        */
    uint8_t  prev_hash[RCF_TIMECHAIN_HASH_LEN];    /* Hash of previous      */
    uint8_t  self_hash[RCF_TIMECHAIN_HASH_LEN];    /* Hash of [0:80]        */
    uint8_t  reserved2[16];                         /* Future use            */
} RCF_Timechain_Entry;

_Static_assert(sizeof(RCF_Timechain_Entry) == RCF_TIMECHAIN_ENTRY_SIZE,
               "RCF_Timechain_Entry must be exactly 128 bytes");

/* ─── Tamper event types ─────────────────────────────────────────────────── */

typedef enum {
    TC_ERR_OK               =  0,
    TC_ERR_NOT_INITIALIZED  = -1,
    TC_ERR_HASH_MISMATCH    = -2,   /* Self-hash verification failed        */
    TC_ERR_CHAIN_BROKEN     = -3,   /* prev_hash link mismatch              */
    TC_ERR_ROLLBACK         = -4,   /* Timestamp moved backward             */
    TC_ERR_CLOCK_ANOMALY    = -5,   /* LSI drift out of tolerance           */
    TC_ERR_RNG_FAIL         = -6,   /* Hardware RNG failure                 */
    TC_ERR_GENESIS_MISSING  = -7,   /* Genesis block absent                 */
    TC_ERR_NEEDS_PROVISION  = -8,   /* First boot, provisioning required    */
} RCF_Timechain_Error;

/* ─── Public API ─────────────────────────────────────────────────────────── */

/* Lifecycle */
RCF_Timechain_Error timechain_init(void);
bool                timechain_is_valid(void);

/* Time operations */
bool timechain_get_timestamp(uint64_t* out_timestamp);
RCF_Timechain_Error timechain_update(void);

/* Genesis (factory provisioning only) */
RCF_Timechain_Error timechain_create_genesis(uint64_t factory_timestamp,
                                              uint32_t power_cycle_count);

/* Tamper detection */
RCF_Timechain_Error timechain_verify_chain(void);
bool                timechain_detect_rollback(uint64_t new_time);
bool                timechain_detect_clock_anomaly(void);
uint32_t            timechain_measure_lsi_hz(void);

/* Diagnostics */
uint32_t timechain_get_monotonic(void);
int32_t  timechain_get_clock_drift_ppm(void);

#ifdef __cplusplus
}
#endif

#endif /* RCF_TIMECHAIN_H */