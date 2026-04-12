/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * License Verification & Code Integrity Engine.
 */

#include "rcf_license.h"
#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_timechain.h"
#include "rcf_pilloff.h"

static RCF_License_Block current_license;
static bool license_valid = false;

bool license_validate(void) {
    // [RCF-START][M-LICENSE-VALIDATE]
    // 1. Verify signature (requires RCF Authority pubkey in vault)
    uint8_t authority_pubkey[32];
    uint32_t keylen;
    if (!vault_load_key(VAULT_KEY_LICENSE_HMAC, authority_pubkey, &keylen)) {
        return false;
    }
    
    uint8_t sig_msg[160]; // payload || hmac
    memcpy(sig_msg, current_license.payload, 128);
    memcpy(sig_msg + 128, current_license.hmac, 32);
    
    if (!ed25519_verify(current_license.signature, sig_msg, 160, authority_pubkey)) {
        // Invalid signature — possible forgery
        trigger_pill_off(PILL_OFF_INVALID_LICENSE);
        return false;
    }
    
    // 2. Verify HMAC (device-bound)
    uint8_t device_hmac[32];
    uint8_t device_secret[32];
    vault_load_key(VAULT_KEY_LICENSE_HMAC, device_secret, &keylen);
    
    hmac_sha256(device_secret, 32, current_license.payload, 128, device_hmac);
    if (memcmp(device_hmac, current_license.hmac, 32) != 0) {
        // License not bound to this device
        trigger_pill_off(PILL_OFF_TAMPER_CLONE);
        return false;
    }
    
    // 3. Decrypt payload
    RCF_License_Payload payload;
    // AES-256-GCM decryption with key from vault
    if (!aes256_gcm_decrypt(device_secret, current_license.payload, 128, (uint8_t*)&payload)) {
        return false;
    }
    
    // 4. Verify code fingerprint
    uint8_t current_fingerprint[32];
    compute_code_fingerprint(current_fingerprint);
    if (memcmp(current_fingerprint, payload.code_fingerprint, 32) != 0) {
        // Code modified since license issuance
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
        license_valid = false; // Graceful degradation, not Pill-Off
        return false;
    }
    
    license_valid = true;
    // [RCF-END]
    return true;
}

static void compute_code_fingerprint(uint8_t* out_hash) {
    // [RCF-START][M-INTEGRITY-CHECK]
    // SHA-256 of .text and .rodata sections
    uint32_t text_start = 0x08000000;  // Flash base
    uint32_t text_size = (uint32_t)&_etext - text_start;
    uint32_t rodata_start = (uint32_t)&_srodata;
    uint32_t rodata_size = (uint32_t)&_erodata - rodata_start;
    
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t*)text_start, text_size);
    sha256_update(&ctx, (uint8_t*)rodata_start, rodata_size);
    
    // Include Option Bytes (configuration)
    uint32_t option_bytes = *(__IO uint32_t*)0x1FFF_C000;
    sha256_update(&ctx, (uint8_t*)&option_bytes, 4);
    
    sha256_final(&ctx, out_hash);
    // [RCF-END]
}

bool license_check_feature(uint8_t feature_flag) {
    if (!license_valid) return false;
    
    // Re-decrypt payload (or cache it securely)
    RCF_License_Payload payload;
    // ... decryption logic (requires device_secret) ...
    
    return (payload.feature_flags & feature_flag) != 0;
}