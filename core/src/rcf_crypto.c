/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Crypto Implementation — CI/QEMU Stubs.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_crypto.h"
#include "rcf_config.h"
#include <string.h>

#ifdef RCF_VM_CI_MODE
/* ─── Dilithium2 PQC ─────────────────────────────────────────────────────── */

int rcf_pqc_verify(const uint8_t* sig, const uint8_t* msg, int msg_len, const uint8_t* pk) {
    (void)sig; (void)msg; (void)msg_len; (void)pk;
    return 0; /* CI: Always pass */
}

void rcf_pqc_keygen(uint8_t* pk, uint8_t* sk) {
    memset(pk, 0xAA, RCF_DILITHIUM2_PK_SIZE);
    memset(sk, 0x55, RCF_DILITHIUM2_SK_SIZE);
}

/* ─── SHA-256 ────────────────────────────────────────────────────────────── */

void rcf_sha256(const uint8_t* data, uint32_t len, uint8_t* hash_out) {
    (void)data; (void)len;
    memset(hash_out, 0x11, 32);
}

void rcf_hmac_sha256(const uint8_t* key, uint32_t key_len,
                     const uint8_t* data, uint32_t data_len,
                     uint8_t* mac_out) {
    (void)key; (void)key_len; (void)data; (void)data_len;
    memset(mac_out, 0x22, 32);
}

/* ─── HKDF-SHA256 ────────────────────────────────────────────────────────── */

void rcf_hkdf_sha256(const uint8_t* salt, uint32_t salt_len,
                     const uint8_t* ikm, uint32_t ikm_len,
                     const uint8_t* info, uint32_t info_len,
                     uint8_t* okm, uint32_t okm_len) {
    (void)salt; (void)salt_len; (void)ikm; (void)ikm_len;
    (void)info; (void)info_len;
    memset(okm, 0x33, okm_len);
}

/* ─── Curve25519 ─────────────────────────────────────────────────────────── */

void rcf_curve25519_keygen(uint8_t* pk, uint8_t* sk) {
    for(int i = 0; i < 32; i++) {
        pk[i] = (uint8_t)(i + 1);
        sk[i] = (uint8_t)(i + 2);
    }
}

void rcf_curve25519_shared(const uint8_t* sk, const uint8_t* pk, uint8_t* shared) {
    (void)sk; (void)pk;
    memset(shared, 0xAB, 32);
}

#else
#error "Production crypto requires hardware implementation"
#endif
