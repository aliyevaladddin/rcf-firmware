// [RCF:RESTRICTED] — Timechain implementation
#include "rcf_timechain.h"
#include "rcf_crypto.h"
#include "rcf_pilloff.h"
#include "rcf_vault.h"
#include "stm32f4xx_hal.h"

#define BACKUP_SRAM_BASE        ((volatile uint8_t*)RCF_BACKUP_SRAM_ADDR)
#define TIMECHAIN_CURRENT       ((volatile RCF_Timechain_Entry*)(BACKUP_SRAM_BASE + RCF_TIMECHAIN_OFFSET))
#define TIMECHAIN_PREVIOUS      ((volatile RCF_Timechain_Entry*)(BACKUP_SRAM_BASE + RCF_TIMECHAIN_OFFSET + 128))
#define TIMECHAIN_GENESIS       ((volatile RCF_Timechain_Entry*)(BACKUP_SRAM_BASE + RCF_GENESIS_OFFSET))

static RTC_HandleTypeDef hrtc;
static uint32_t last_lsi_measurement = 32000;
static bool timechain_valid = false;

bool timechain_init(void) {
    // Enable Backup SRAM clock
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    
    // Enable RTC
    __HAL_RCC_RTC_ENABLE();
    
    // Check if RTC is initialized
    if (__HAL_RTC_IS_INITIALIZED(&hrtc)) {
        // RTC running — verify timechain continuity
        if (TIMECHAIN_CURRENT->self_hash[0] != 0) {
            // Verify chain integrity
            uint8_t computed_hash[32];
            sha256_hw((uint8_t*)TIMECHAIN_CURRENT, offsetof(RCF_Timechain_Entry, self_hash), computed_hash);
            
            if (memcmp(computed_hash, (void*)TIMECHAIN_CURRENT->self_hash, 32) != 0) {
                // Chain corrupted — possible tamper
                trigger_pill_off(PILL_OFF_TAMPER_TIME);
                return false;
            }
            
            // Verify link to previous
            if (TIMECHAIN_PREVIOUS->self_hash[0] != 0) {
                if (memcmp(TIMECHAIN_CURRENT->prev_hash, (void*)TIMECHAIN_PREVIOUS->self_hash, 32) != 0) {
                    trigger_pill_off(PILL_OFF_TAMPER_TIME);
                    return false;
                }
            }
            
            timechain_valid = true;
        } else {
            // First boot or backup lost — check genesis
            if (TIMECHAIN_GENESIS->self_hash[0] != 0) {
                // Genesis exists but current missing — rollback attack
                trigger_pill_off(PILL_OFF_TAMPER_ROLLBACK);
                return false;
            }
            // True first boot — needs provisioning
            return false;
        }
    } else {
        // RTC not running — power loss or tamper
        // Check if genesis exists
        if (TIMECHAIN_GENESIS->self_hash[0] == 0) {
            return false; // Needs provisioning
        }
        
        // Attempt to restore from backup
        // If VBAT was removed, genesis will be lost → rollback detected
        trigger_pill_off(PILL_OFF_TAMPER_ROLLBACK);
        return false;
    }
    
    return true;
}

bool timechain_update(void) {
    if (!timechain_valid) return false;
    
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    
    uint64_t new_timestamp = rtc_to_unix(sTime, sDate);
    
    // Monotonicity check
    if (new_timestamp < TIMECHAIN_CURRENT->timestamp_unix) {
        // Time went backwards — rollback attack
        if (timechain_detect_rollback(new_timestamp)) {
            trigger_pill_off(PILL_OFF_TAMPER_TIME);
            return false;
        }
    }
    
    // Create new entry
    RCF_Timechain_Entry new_entry;
    new_entry.timestamp_unix = new_timestamp;
    new_entry.monotonic_counter = TIMECHAIN_CURRENT->monotonic_counter + 1;
    new_entry.subsecond_ticks = RTC->SSR; // Sub-second register
    
    // Add entropy
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)new_entry.entropy_pool);
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(new_entry.entropy_pool + 4));
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(new_entry.entropy_pool + 8));
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(new_entry.entropy_pool + 12));
    
    // Link to previous
    memcpy(new_entry.prev_hash, (void*)TIMECHAIN_CURRENT->self_hash, 32);
    
    // Metadata
    new_entry.power_cycle_count = TIMECHAIN_CURRENT->power_cycle_count;
    new_entry.temperature_celsius = get_internal_temperature();
    new_entry.voltage_mv = get_vbat_voltage();
    
    // Self-hash
    sha256_hw((uint8_t*)&new_entry, offsetof(RCF_Timechain_Entry, self_hash), new_entry.self_hash);
    
    // Shift: current → previous, new → current
    memcpy((void*)TIMECHAIN_PREVIOUS, (void*)TIMECHAIN_CURRENT, sizeof(RCF_Timechain_Entry));
    memcpy((void*)TIMECHAIN_CURRENT, &new_entry, sizeof(RCF_Timechain_Entry));
    
    return true;
}

bool timechain_detect_clock_anomaly(void) {
    // Measure LSI frequency via HSE reference
    uint32_t lsi_freq = measure_lsi_frequency();
    
    // Check for significant drift
    int32_t drift = ((int32_t)lsi_freq - 32000) * 1000 / 32000; // ppm
    
    if (drift < -RCF_MAX_TIME_DRIFT_PPM || drift > RCF_MAX_TIME_DRIFT_PPM) {
        // Clock anomaly detected
        rcf_audit_log(EVENT_CLOCK_ANOMALY, drift);
        
        if (drift < -5000) {
            // Significant slowing — possible attack
            trigger_pill_off(PILL_OFF_TAMPER_CLOCK);
            return true;
        }
    }
    
    last_lsi_measurement = lsi_freq;
    return false;
}

static uint32_t measure_lsi_frequency(void) {
    // TIM5 CH4 input capture on LSI, clocked by HSE/8 = 1MHz
    // Capture interval: 100ms
    // LSI ticks = capture_value, expected ~3200
    
    TIM5->CCER &= ~TIM_CCER_CC4E;  // Disable capture
    TIM5->CCMR2 = TIM_CCMR2_CC4S_0; // IC4 on TI4
    TIM5->CCER |= TIM_CCER_CC4E;   // Enable capture
    
    uint32_t cap1 = TIM5->CCR4;
    HAL_Delay(100);
    uint32_t cap2 = TIM5->CCR4;
    
    uint32_t lsi_ticks = cap2 - cap1;
    // lsi_freq = lsi_ticks * 10 (since 100ms = 0.1s)
    return lsi_ticks * 10;
}