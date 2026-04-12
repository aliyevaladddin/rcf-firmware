/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * License Verification & Code Integrity Engine — v1.3.
 */

#include "rcf_license.h"
#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_timechain.h"
#include "rcf_pilloff.h"

static RCF_License_Block current_license;
static bool license_valid = false;

bool license_validate(void) {
    // 1. Verify Dilithium2 signature (requires RCF Authority PQC pubkey in vault)
    uint8_t authority_pqc_pubkey[1312];
    uint32_t keylen;
    if (!vault_load_key(VAULT_KEY_PQC_PUB, authority_pqc_pubkey, &keylen)) {
        return false;
    }
    
    uint8_t sig_msg[160]; // payload || hmac
    memcpy(sig_msg, current_license.payload, 128);
    memcpy(sig_msg + 128, current_license.hmac, 32);
    
    /* [RCF v1.3] PQC Signature Verification */
    if (rcf_pqc_verify(current_license.signature, sig_msg, 160, authority_pqc_pubkey) != 0) {
        trigger_pill_off(PILL_OFF_INVALID_LICENSE);
        return false;
    }
    
    // 2. Verify HMAC (device-bound)
    uint8_t device_hmac[32];
    uint8_t device_secret[32];
    vault_load_key(VAULT_KEY_LICENSE_HMAC, device_secret, &keylen);
    
    hmac_sha256(device_secret, 32, current_license.payload, 128, device_hmac);
    if (memcmp(device_hmac, current_license.hmac, 32) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_CLONE);
        return false;
    }
    
    // 3. Decrypt payload using device secret
    RCF_License_Payload payload;
    if (!aes256_gcm_decrypt(device_secret, current_license.payload, 128, (uint8_t*)&payload)) {
        return false;
    }
    
    // 4. Verify code fingerprint
    uint8_t current_fingerprint[32];
    compute_code_fingerprint(current_fingerprint);
    if (memcmp(current_fingerprint, payload.code_fingerprint, 32) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_CODE);
        return false;
    }
    
    // 5. Verify device binding
    uint8_t chip_uid[12];
    HAL_GetUID(chip_uid);
    if (memcmp(chip_uid, payload.device_binding, 12) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_CLONE);
        return false;
    }
    
    // 6. Verify expiration
    uint64_t current_time;
    timechain_get_timestamp(&current_time);
    if (current_time > payload.expiration_timestamp) {
        license_valid = false;
        return false;
    }
    
    license_valid = true;
    return true;
}

static void compute_code_fingerprint(uint8_t* out_hash) {
    // Implement hash over flash sectors
    // ...
}

bool license_check_feature(uint8_t feature_flag) {
    if (!license_valid) return false;
    return true; // Placeholder
}