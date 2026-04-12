/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Crypto Implementation — CI/QEMU Stubs.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_crypto.h"
#include "rcf_config.h"

#ifdef RCF_VM_CI_MODE
/* CI stubs — no real crypto, just pass-through for testing */

int rcf_pqc_verify(const uint8_t* sig, const uint8_t* msg, int msg_len, const uint8_t* pk) {
    (void)sig; (void)msg; (void)msg_len; (void)pk;
    return 0; /* CI: always pass verification */
}

void rcf_pqc_keygen(uint8_t* pk, uint8_t* sk) {
    for (int i = 0; i < (int)RCF_DILITHIUM2_PK_SIZE; i++) pk[i] = i & 0xFF;
    for (int i = 0; i < (int)RCF_DILITHIUM2_SK_SIZE; i++) sk[i] = (i + 128) & 0xFF;
}

void rcf_sha256(const uint8_t* data, uint32_t len, uint8_t* hash_out) {
    /* CI: fake hash — NOT for production */
    for (int i = 0; i < 32; i++) hash_out[i] = i;
    (void)data; (void)len;
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

#else
/* Production Implementation Placeholder */
#error "Production crypto requires hardware implementation — Define RCF_VM_CI_MODE for testing"
#endif
