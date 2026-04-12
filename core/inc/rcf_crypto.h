/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Cryptographic Engine — PQC & Classical.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_CRYPTO_H
#define RCF_CRYPTO_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ─── Dilithium2 PQC (FIPS 204) ───────────────────────────────────────── */

#define RCF_DILITHIUM2_SIG_SIZE     2420U
#define RCF_DILITHIUM2_PK_SIZE      1312U
#define RCF_DILITHIUM2_SK_SIZE      2528U

/* Verify Dilithium2 signature against public key */
int rcf_pqc_verify(const uint8_t* sig, const uint8_t* msg, int msg_len, const uint8_t* pk);

/* Generate keypair (Bunker mode only) */
void rcf_pqc_keygen(uint8_t* pk, uint8_t* sk);

/* ─── SHA-256 (Hardware Accelerated) ──────────────────────────────────── */

void rcf_sha256_init(void);
void rcf_sha256_update(const uint8_t* data, uint32_t len);
void rcf_sha256_final(uint8_t* hash_out);  /* 32 bytes */

/* One-shot SHA-256 */
void rcf_sha256(const uint8_t* data, uint32_t len, uint8_t* hash_out);

/* ─── AES-256-GCM ─────────────────────────────────────────────────────── */

void rcf_aes256_gcm_encrypt(const uint8_t* key, const uint8_t* iv, 
                            const uint8_t* plaintext, uint32_t len,
                            uint8_t* ciphertext, uint8_t* tag);

bool rcf_aes256_gcm_decrypt(const uint8_t* key, const uint8_t* iv,
                            const uint8_t* ciphertext, uint32_t len,
                            const uint8_t* tag, uint8_t* plaintext);

/* ─── HMAC-SHA256 ─────────────────────────────────────────────────────── */

void rcf_hmac_sha256(const uint8_t* key, uint32_t key_len,
                     const uint8_t* data, uint32_t data_len,
                     uint8_t* mac_out);  /* 32 bytes */

/* ─── Curve25519 ECDH ─────────────────────────────────────────────────── */

void rcf_curve25519_keygen(uint8_t* pk, uint8_t* sk);
void rcf_curve25519_shared(const uint8_t* sk, const uint8_t* pk, uint8_t* shared);

#endif /* RCF_CRYPTO_H */
