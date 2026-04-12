/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * Pulse Sensor — CI/QEMU Stub Implementation.
 */

#include "rcf_pulse.h"
#include "rcf_audit.h"

#ifdef RCF_VM_CI_MODE
/* QEMU simulation: mock biometric data */
static uint8_t mock_pulse_counter = 0;

bool pulse_init(void) {
    rcf_audit_log(EVENT_PULSE_INIT, 0);
    return true;
}


bool pulse_check_activation(void) {
    /* CI mode: auto-trigger every 10th call */
    if (++mock_pulse_counter >= 10) {
        mock_pulse_counter = 0;
        return true;
    }
    return false;
}

bool pulse_validate_liveness(void) {
    /* CI mode: always pass */
    rcf_audit_log(EVENT_PULSE_LIVENESS, 1);
    return true;
}


uint8_t get_failed_attempts(void) {
    return 0; /* CI: no failures */
}

void pulse_reset_auth(void) {
    mock_pulse_counter = 0;
}

#else
/* Production: hardware implementation placeholder */
bool pulse_init(void) { return false; }
bool pulse_check_activation(void) { return false; }
bool pulse_validate_liveness(void) { return false; }
uint8_t get_failed_attempts(void) { return 0; }
void pulse_reset_auth(void) {}
#endif
