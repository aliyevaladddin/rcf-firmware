/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Crypto Implementation — CI/QEMU Stubs.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_crypto.h"
#include "rcf_config.h"
#include <string.h>

#ifdef RCF_VM_CI_MODE
/* CI stubs — no real crypto, just pass-through for testing */

void rcf_pqc_keygen(uint8_t* pk, uint8_t* sk) {

    for (int i = 0; i < (int)RCF_DILITHIUM2_PK_SIZE; i++) pk[i] = i & 0xFF;
    for (int i = 0; i < (int)RCF_DILITHIUM2_SK_SIZE; i++) sk[i] = (i + 128) & 0xFF;
}

void rcf_sha256(const uint8_t* data, uint32_t len, uint8_t* hash_out) {
    /* CI: fake hash — NOT for production */
    for (int i = 0; i < 32; i++) hash_out[i] = i;
    (void)data; (void)len;
}

void rcf_aes256_gcm_encrypt(const uint8_t* key, const uint8_t* iv,
                            const uint8_t* plaintext, uint32_t len,
                            uint8_t* ciphertext, uint8_t* tag) {
    (void)key; (void)iv; (void)tag;
    /* CI: plaintext pass-through */
    memcpy(ciphertext, plaintext, len);
}

bool rcf_aes256_gcm_decrypt(const uint8_t* key, const uint8_t* iv,
                            const uint8_t* ciphertext, uint32_t len,
                            const uint8_t* tag, uint8_t* plaintext) {
    (void)key; (void)iv; (void)tag;
    memcpy(plaintext, ciphertext, len);
    return true; /* CI: always pass */
}

void rcf_hmac_sha256(const uint8_t* key, uint32_t key_len,
                     const uint8_t* data, uint32_t data_len,
                     uint8_t* mac_out) {
    (void)key; (void)key_len; (void)data; (void)data_len;
    for (int i = 0; i < 32; i++) mac_out[i] = i;
}

void rcf_curve25519_keygen(uint8_t* pk, uint8_t* sk) {
    /* CI: deterministic mock keys */
    for(int i = 0; i < 32; i++) {
        pk[i] = (uint8_t)(i * 7 + 3);
        sk[i] = (uint8_t)(i * 5 + 11);
    }
}

    /* CI: mock shared secret */
    for(int i = 0; i < 32; i++) {
        shared[i] = 0xAB;
    }
}

void rcf_hkdf_sha256(const uint8_t* salt, uint32_t salt_len,
                     const uint8_t* ikm, uint32_t ikm_len,
                     const uint8_t* info, uint32_t info_len,
                     uint8_t* okm, uint32_t okm_len) {
    /* CI: Simulating HKDF-SHA256 for testing */
    uint8_t prk[32];
    rcf_hmac_sha256(salt, salt_len, ikm, ikm_len, prk);
    
    /* Simplified expansion for CI: fills OKM with PRK-derived data */
    (void)info; (void)info_len;
    for (uint32_t i = 0; i < okm_len; i++) {
        okm[i] = prk[i % 32] ^ (uint8_t)i;
    }
}

#else
/* Production Implementation Placeholder */
#error "Production crypto requires hardware implementation — Define RCF_VM_CI_MODE for testing"
#endif
