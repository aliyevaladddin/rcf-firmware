/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * Main entry point — Lume Firmware.
 */
// [RCF:PUBLIC] — Main entry point
#include "main.h"
#include "rcf_config.h"
#include "rcf_vault.h"
#include "rcf_timechain.h"
#include "rcf_license.h"
#include "rcf_protocol.h"
#include "rcf_pulse.h"
#include "rcf_led.h"
#include "rcf_pilloff.h"
#include "rcf_modules.h"
#include "rcf_vm.h"


int main(void) {
    // HAL initialization
    HAL_Init();
    SystemClock_Config();
    
    // LED initialization (immediate user feedback)
    led_init();
    led_set_pattern(PATTERN_BOOT);
    rcf_vm_execute("BOOT_IDENTITY", ACODE_IDENTITY, ACODE_IDENTITY_SIZE);

    
    // TRNG initialization (critical for security)
    hrng.Instance = RNG;
    HAL_RNG_Init(&hrng);
    
    // 1. Tamper detection (before anything else)
    tamper_init();
    
    // 2. Vault initialization
    if (!vault_init()) {
        // Unprovisioned device — enter provisioning mode
        led_set_pattern(PATTERN_PROVISIONING);
        enter_provisioning_mode();
    }
    
    // 3. Timechain initialization
    if (!timechain_init()) {
        // Time integrity compromised
        trigger_pill_off(PILL_OFF_TAMPER_TIME);
    }
    
    // 4. License validation
    if (!license_validate()) {
        // License invalid or expired
        if (!license_is_valid()) {
            // Graceful degradation: only PUBLIC features
            led_set_pattern(PATTERN_LICENSE_INVALID);
        }
    }
    
    // 5. USB CDC initialization
    usb_init();
    protocol_init();
    
    // 6. Pulse sensor initialization
    pulse_init();
    
    // 7. Main loop
    led_set_pattern(PATTERN_NEURAL_CYAN);
    
    while (1) {
        // Process USB commands
        protocol_process();
        
        // Check pulse sensor
        if (pulse_check_activation()) {
            if (pulse_validate_liveness()) {
                // Valid biometric — attempt session establishment
                if (protocol_establish_session()) {
                    led_set_pattern(PATTERN_SESSION_ACTIVE);
                    deploy_dos_environment();
                }
            } else {
                // Failed liveness check
                rcf_audit_log(EVENT_BIOMETRIC_FAIL, 0);
                if (get_failed_attempts() >= RCF_MAX_AUTH_ATTEMPTS) {
                    trigger_pill_off(PILL_OFF_INVALID_AUTH);
                }
            }
        }
        
        // Periodic timechain update
        static uint32_t last_update = 0;
        if (HAL_GetTick() - last_update > RCF_TIMECHAIN_INTERVAL_MS) {
            timechain_update();
            timechain_detect_clock_anomaly();
            last_update = HAL_GetTick();
        }
        
        // Watchdog refresh (if system hangs, Pill-Off triggers)
        HAL_IWDG_Refresh(&hiwdg);
    }
}

void deploy_dos_environment(void) {
    // Signal host to load Aurora Access dOS
    protocol_send_dos_bootstrap();
    
    // Transfer control to session handler
    while (protocol_is_session_active()) {
        protocol_process();
        
        // Handle dOS requests
        uint8_t request[256];
        uint16_t received;
        if (protocol_receive_data(request, sizeof(request), &received)) {
            process_dos_request(request, received);
        }
    }
    
    // Session ended — cleanup
    led_set_pattern(PATTERN_NEURAL_CYAN);
    secure_memzero(&session, sizeof(session));
}

void Error_Handler(void) {
    // Unhandled error — treat as potential attack
    trigger_pill_off(PILL_OFF_WATCHDOG);
}