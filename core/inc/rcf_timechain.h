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

#define RCF_LSI_NOMINAL_HZ          32000U

/* Anti-rollback: max tolerated backwards delta before alarm */
#define RCF_MAX_BACKWARD_DELTA_SEC  5U

/* ─── Timechain entry ────────────────────────────────────────────────────── */

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
    TC_ERR_HASH_MISMATCH    = -2,
    TC_ERR_CHAIN_BROKEN     = -3,
    TC_ERR_ROLLBACK         = -4,
    TC_ERR_CLOCK_ANOMALY    = -5,
    TC_ERR_RNG_FAIL         = -6,
    TC_ERR_GENESIS_MISSING  = -7,
    TC_ERR_NEEDS_PROVISION  = -8,
} RCF_Timechain_Error;

/* ─── Public API ─────────────────────────────────────────────────────────── */

RCF_Timechain_Error timechain_init(void);
bool                timechain_is_valid(void);
bool timechain_get_timestamp(uint64_t* out_timestamp);
RCF_Timechain_Error timechain_update(void);
RCF_Timechain_Error timechain_create_genesis(uint64_t factory_timestamp,
                                              uint32_t power_cycle_count);
RCF_Timechain_Error timechain_verify_chain(void);
bool                timechain_detect_rollback(uint64_t new_time);
bool                timechain_detect_clock_anomaly(void);
uint32_t            timechain_measure_lsi_hz(void);
uint32_t timechain_get_monotonic(void);
int32_t  timechain_get_clock_drift_ppm(void);

#ifdef __cplusplus
}
#endif

#endif /* RCF_TIMECHAIN_H */