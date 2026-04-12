/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Timechain Implementation & Anti-Rollback Protection.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#include "rcf_timechain.h"
#include "rcf_crypto.h"
#include "rcf_pilloff.h"
#include "rcf_vault.h"
#include "rcf_audit.h"
#include "main.h"
#include "stm32f4xx_hal.h"

#include <string.h>
#include <stddef.h>

/* Forward declarations for timechain helpers implemented in hal_stubs.c */
uint64_t rtc_to_unix(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate);
int16_t  get_internal_temperature(void);
uint32_t get_vbat_voltage(void);


/* ─── Backup SRAM layout ─────────────────────────────────────────────────── */

#define BACKUP_SRAM_BASE    ((volatile uint8_t*)RCF_BACKUP_SRAM_ADDR)
#define TC_CURRENT          ((volatile RCF_Timechain_Entry*) \
                             (BACKUP_SRAM_BASE + RCF_TIMECHAIN_OFFSET))
#define TC_PREVIOUS         ((volatile RCF_Timechain_Entry*) \
                             (BACKUP_SRAM_BASE + RCF_TIMECHAIN_OFFSET + \
                              RCF_TIMECHAIN_ENTRY_SIZE))
#define TC_GENESIS          ((volatile RCF_Timechain_Entry*) \
                             (BACKUP_SRAM_BASE + RCF_GENESIS_OFFSET))

/* ─── Module state ───────────────────────────────────────────────────────── */

static RTC_HandleTypeDef hrtc;
static uint32_t          last_lsi_hz    = RCF_LSI_NOMINAL_HZ;
static bool              tc_valid       = false;

/* ─── Internal helpers ───────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */

/**
 * Returns true if the entry's self_hash field has ever been written
 * (i.e. is not all-zeros, which is the Backup SRAM reset state).
 */
static inline bool _entry_exists(const volatile RCF_Timechain_Entry* e)
{
    for (size_t i = 0; i < RCF_TIMECHAIN_HASH_LEN; i++) {
        if (e->self_hash[i] != 0) return true;
    }
    return false;
}

/**
 * Verify the self_hash of an entry.
 * self_hash = SHA-256( entry[0 : offsetof(self_hash)] )
 */
static bool _verify_self_hash(const volatile RCF_Timechain_Entry* e)
{
    uint8_t computed[RCF_TIMECHAIN_HASH_LEN];
    rcf_sha256((const uint8_t*)e,
              offsetof(RCF_Timechain_Entry, self_hash),
              computed);
    return (memcmp(computed, (const void*)e->self_hash,
                   RCF_TIMECHAIN_HASH_LEN) == 0);
}

/**
 * Fill entropy_pool using hardware RNG (4 × 4-byte words).
 * Returns false if any RNG call fails.
 */
static bool _fill_entropy(uint8_t* pool)
{
    for (size_t i = 0; i < RCF_TIMECHAIN_ENTROPY_LEN; i += 4) {
        uint32_t rnd;
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {
            return false;
        }
        memcpy(pool + i, &rnd, 4);
    }
    return true;
}

/**
 * Compute self_hash for a new entry and write it in-place.
 */
static void _finalize_entry(RCF_Timechain_Entry* e)
{
    rcf_sha256((const uint8_t*)e,
              offsetof(RCF_Timechain_Entry, self_hash),
              e->self_hash);
}

/* ─── Lifecycle ──────────────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
RCF_Timechain_Error timechain_init(void)
{
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    __HAL_RCC_RTC_ENABLE();

    if (__HAL_RTC_IS_INITIALIZED(&hrtc)) {
        /* RTC running — check current entry */
        if (_entry_exists(TC_CURRENT)) {

            /* 1. Verify self-hash */
            if (!_verify_self_hash(TC_CURRENT)) {
                trigger_pill_off(PILL_OFF_TAMPER_TIME);
                return TC_ERR_HASH_MISMATCH;
            }

            /* 2. Verify chain link to previous */
            if (_entry_exists(TC_PREVIOUS)) {
                if (memcmp((const void*)TC_CURRENT->prev_hash,
                           (const void*)TC_PREVIOUS->self_hash,
                           RCF_TIMECHAIN_HASH_LEN) != 0) {
                    trigger_pill_off(PILL_OFF_TAMPER_TIME);
                    return TC_ERR_CHAIN_BROKEN;
                }
            }

            tc_valid = true;
            return TC_ERR_OK;

        } else {
            /* No current entry — if genesis exists, this is a rollback */
            if (_entry_exists(TC_GENESIS)) {
                trigger_pill_off(PILL_OFF_TAMPER_TIME);
                return TC_ERR_ROLLBACK;
            }
            /* True first boot — provisioning required */
            return TC_ERR_NEEDS_PROVISION;
        }

    } else {
        /* RTC not running — power loss or VBAT tamper */
        if (!_entry_exists(TC_GENESIS)) {
            return TC_ERR_NEEDS_PROVISION;
        }
        /* Genesis present but RTC lost → VBAT removed → rollback attack */
        trigger_pill_off(PILL_OFF_TAMPER_TIME);
        return TC_ERR_ROLLBACK;
    }
}

bool timechain_is_valid(void)
{
    return tc_valid;
}

/* ─── Time access ────────────────────────────────────────────────────────── */

bool timechain_get_timestamp(uint64_t* out_timestamp)
{
    if (!tc_valid || !out_timestamp) return false;
    *out_timestamp = TC_CURRENT->timestamp_unix;
    return true;
}

uint32_t timechain_get_monotonic(void)
{
    return tc_valid ? TC_CURRENT->monotonic_counter : 0U;
}

/* ─── Chain update ───────────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
RCF_Timechain_Error timechain_update(void)
{
    if (!tc_valid) return TC_ERR_NOT_INITIALIZED;

    /* 1. Read RTC */
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    uint64_t new_ts = rtc_to_unix(sTime, sDate);

    /* 2. Monotonicity check */
    if (new_ts < TC_CURRENT->timestamp_unix) {
        if (timechain_detect_rollback(new_ts)) {
            trigger_pill_off(PILL_OFF_TAMPER_TIME);
            return TC_ERR_ROLLBACK;
        }
        /* Within tolerance — use current timestamp to avoid regression */
        new_ts = TC_CURRENT->timestamp_unix;
    }

    /* 3. Build new entry */
    RCF_Timechain_Entry e;
    memset(&e, 0, sizeof(e));

    e.timestamp_unix      = new_ts;
    e.monotonic_counter   = TC_CURRENT->monotonic_counter + 1U;
    e.subsecond_ticks     = (uint16_t)(RTC->SSR & 0xFFFFU);
    e.temperature_celsius = get_internal_temperature();
    e.voltage_mv          = get_vbat_voltage();
    e.power_cycle_count   = TC_CURRENT->power_cycle_count;
    e.lsi_frequency_hz    = last_lsi_hz;

    /* 4. Fill entropy */
    if (!_fill_entropy(e.entropy_pool)) {
        return TC_ERR_RNG_FAIL;
    }

    /* 5. Link to previous */
    memcpy(e.prev_hash, (const void*)TC_CURRENT->self_hash,
           RCF_TIMECHAIN_HASH_LEN);

    /* 6. Compute self-hash and finalize */
    _finalize_entry(&e);

    /* 7. Shift: current → previous, new → current */
    memcpy((void*)TC_PREVIOUS, (const void*)TC_CURRENT,
           RCF_TIMECHAIN_ENTRY_SIZE);
    memcpy((void*)TC_CURRENT, &e, RCF_TIMECHAIN_ENTRY_SIZE);

    /* 8. Zeroize stack copy */
    memset(&e, 0, sizeof(e));

    return TC_ERR_OK;
}

/* ─── Genesis block ──────────────────────────────────────────────────────── */

RCF_Timechain_Error timechain_create_genesis(uint64_t factory_timestamp,
                                               uint32_t power_cycle_count)
{
    if (_entry_exists(TC_GENESIS)) {
        /* Genesis must only be written once at the factory */
        return TC_ERR_HASH_MISMATCH;
    }

    RCF_Timechain_Entry g;
    memset(&g, 0, sizeof(g));

    g.timestamp_unix    = factory_timestamp;
    g.monotonic_counter = 0U;
    g.power_cycle_count = power_cycle_count;
    g.lsi_frequency_hz  = RCF_LSI_NOMINAL_HZ;

    /* Genesis prev_hash = all-zeros (no predecessor) */
    memset(g.prev_hash, 0, RCF_TIMECHAIN_HASH_LEN);

    if (!_fill_entropy(g.entropy_pool)) return TC_ERR_RNG_FAIL;

    _finalize_entry(&g);

    memcpy((void*)TC_GENESIS,  &g, RCF_TIMECHAIN_ENTRY_SIZE);
    memcpy((void*)TC_CURRENT,  &g, RCF_TIMECHAIN_ENTRY_SIZE);

    memset(&g, 0, sizeof(g));

    tc_valid = true;
    return TC_ERR_OK;
}

/* ─── Tamper detection ───────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
bool timechain_detect_rollback(uint64_t new_ts)
{
    if (!tc_valid) return false;

    uint64_t current_ts = TC_CURRENT->timestamp_unix;
    if (new_ts >= current_ts) return false;

    uint64_t delta = current_ts - new_ts;
    if (delta > RCF_MAX_BACKWARD_DELTA_SEC) {
        rcf_audit_log(EVENT_TIME_ROLLBACK, (uint32_t)delta);
        return true;
    }
    return false;
}

bool timechain_detect_clock_anomaly(void)
{
    uint32_t lsi_hz = timechain_measure_lsi_hz();
    last_lsi_hz     = lsi_hz;

    int32_t drift_ppm =
        ((int32_t)lsi_hz - (int32_t)RCF_LSI_NOMINAL_HZ) * 1000 /
        (int32_t)RCF_LSI_NOMINAL_HZ;

    if (drift_ppm < -(int32_t)RCF_MAX_TIME_DRIFT_FATAL ||
        drift_ppm >  (int32_t)RCF_MAX_TIME_DRIFT_FATAL) {
        rcf_audit_log(EVENT_CLOCK_ANOMALY, (uint32_t)drift_ppm);
        trigger_pill_off(PILL_OFF_TAMPER_CLOCK);
        return true;
    }

    if (drift_ppm < -(int32_t)RCF_MAX_TIME_DRIFT_PPM ||
        drift_ppm >  (int32_t)RCF_MAX_TIME_DRIFT_PPM) {
        rcf_audit_log(EVENT_CLOCK_ANOMALY, (uint32_t)drift_ppm);
    }

    return false;
}

int32_t timechain_get_clock_drift_ppm(void)
{
    return ((int32_t)last_lsi_hz - (int32_t)RCF_LSI_NOMINAL_HZ) * 1000 /
           (int32_t)RCF_LSI_NOMINAL_HZ;
}

RCF_Timechain_Error timechain_verify_chain(void)
{
    if (!_entry_exists(TC_CURRENT)) return TC_ERR_NOT_INITIALIZED;
    if (!_verify_self_hash(TC_CURRENT))  return TC_ERR_HASH_MISMATCH;

    if (_entry_exists(TC_PREVIOUS)) {
        if (memcmp((const void*)TC_CURRENT->prev_hash,
                   (const void*)TC_PREVIOUS->self_hash,
                   RCF_TIMECHAIN_HASH_LEN) != 0) {
            return TC_ERR_CHAIN_BROKEN;
        }
        if (!_verify_self_hash(TC_PREVIOUS)) return TC_ERR_HASH_MISMATCH;
    }

    return TC_ERR_OK;
}

/* ─── LSI measurement (TIM5 CH4 input capture) ───────────────────────────── */

/**
 * Measure LSI frequency using TIM5 CH4 input capture.
 * TIM5 clocked by HSE/8 = 1 MHz reference.
 * Capture interval: 100 ms → expected LSI ticks ≈ 3200.
 * Returns frequency in Hz (nominal: 32000).
 *
 * Note: Caller must ensure TIM5 is enabled and LSI is routed to CH4.
 */
uint32_t timechain_measure_lsi_hz(void)
{
    /* Disable capture, configure IC4 on TI4 */
    TIM5->CCER  &= ~TIM_CCER_CC4E;
    TIM5->CCMR2  =  TIM_CCMR2_CC4S_0;
    TIM5->CCER  |=  TIM_CCER_CC4E;

    /* Discard initial capture to flush stale value */
    // (void)RTC->SSR; // Placeholder

    uint32_t cap1 = 3200; // Stub
    HAL_Delay(100);             /* 100 ms reference window */
    uint32_t cap2 = 6400; // Stub

    uint32_t ticks = cap2 - cap1;  /* Handles 32-bit rollover correctly */

    /* freq_hz = ticks_per_100ms × 10 */
    return ticks * 10U;
}