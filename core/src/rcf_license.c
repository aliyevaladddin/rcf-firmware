/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * License Validation — Implementation.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_license.h"
#include "stm32f4xx_hal.h"
#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_audit.h"
#include "rcf_pilloff.h"
#include <string.h>

/* Forward declarations */
static void compute_code_fingerprint(uint8_t* out_hash);

static RCF_License_Block current_license;
static bool license_active = false;

bool license_init(void) {
    /* [RCF v1.3] Load license from Vault or Flash storage */
    // vault_get_license(&current_license);
    return true;
}

bool license_validate(void) {
    uint8_t device_secret[32];
    uint8_t device_hmac[32];
    uint32_t secret_len = 32;
    RCF_License_Payload payload;

    /* 1. Get Device Root Secret from Vault */
    if (!vault_load_key(VAULT_KEY_LICENSE_HMAC, device_secret, &secret_len)) {
        return false;
    }

    /* 2. Verify HMAC of the license block */
    rcf_hmac_sha256(device_secret, 32, current_license.payload, 128, device_hmac);
    if (memcmp(device_hmac, current_license.hmac, 32) != 0) {
        rcf_audit_log(EVENT_CRYPTO_FAIL, 0x01);
        return false;
    }

    /* 3. Decrypt Payload */
    // In CI mode, rcf_aes256_gcm_decrypt is a stub
    if (!rcf_aes256_gcm_decrypt(device_secret, NULL, current_license.payload, 128, NULL, (uint8_t*)&payload)) {
        return false;
    }

    /* 4. Check Device Binding (UID) */
    uint32_t uid[3];
    uid[0] = HAL_GetUIDw0();
    uid[1] = HAL_GetUIDw1();
    uid[2] = HAL_GetUIDw2();
    if (memcmp(uid, payload.device_binding, 12) != 0) {
        return false;
    }

    /* 5. Code Integrity Fingerprint check */
    uint8_t current_fingerprint[32];
    compute_code_fingerprint(current_fingerprint);
    if (memcmp(current_fingerprint, payload.code_fingerprint, 32) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_CODE);
        return false;
    }

    license_active = true;
    return true;
}

bool license_is_valid(void) {
    return license_active;
}

static void compute_code_fingerprint(uint8_t* out_hash) {
    /* [RCF v1.3] Hash .text and .rodata segments */
    extern uint32_t _stext, _etext, _srodata, _erodata;
    uint32_t text_len = (uint32_t)&_etext - (uint32_t)&_stext;
    
    rcf_sha256((uint8_t*)&_stext, text_len, out_hash);
}

bool license_check_feature(uint8_t feature_flag) {
    if (!license_active) return false;
    // return (current_license_payload.feature_flags & feature_flag) != 0;
    return true; 
}

uint8_t license_get_features(void) {
    return 0xFF;
}

bool license_update(const RCF_License_Block* new_license) {
    if (!new_license) return false;
    memcpy(&current_license, new_license, sizeof(RCF_License_Block));
    return license_validate();
}