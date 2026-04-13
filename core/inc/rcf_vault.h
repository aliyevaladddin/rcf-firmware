/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Secure Key Vault & Anti-Tamper Storage Definitions.
 */

#ifndef RCF_VAULT_H
#define RCF_VAULT_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>

/* [RCF v1.3] Production Build Anchors */
#define RCF_MEK_OTP_ADDR    0x1FFF7A00  // 32 bytes MEK in OTP
#define RCF_MPK_FLASH_ADDR  0x08004000  // Encrypted Dilithium2 PK (1312B) in Flash


typedef enum {
    VAULT_KEY_DEVICE_PRIV       = 0,   // Curve25519 static key
    VAULT_KEY_LICENSE_HMAC      = 1,   // License validation key
    VAULT_KEY_SESSION_EPHEMERAL = 2,   // Ephemeral per-session
    VAULT_KEY_AUDIT_SIGN        = 3,   // Ed25519 audit signing
    VAULT_KEY_PQC_PRIV          = 4,   // Dilithium2 Private Key
    VAULT_KEY_PQC_PUB           = 5,   // Dilithium2 Public Key
    VAULT_KEY_COUNT             = 6
} Vault_KeyType;

typedef struct {
    uint8_t  key_id;
    uint8_t  key_data[2420 + 1312];  // Max possible size (Dilithium2/v1.3)
    uint32_t key_attributes;         // Permissions, lifetime
    uint8_t  key_checksum[32];      // SHA-256 for integrity
} Vault_KeyEntry;

// Lifecycle
bool vault_init(void);
bool vault_factory_reset(void);
bool vault_is_initialized(void);

// Key operations
bool vault_load_key(Vault_KeyType type, uint8_t* out_buffer, uint32_t* out_len);
bool vault_generate_ephemeral(uint8_t* out_pubkey, uint8_t* out_privkey);
bool vault_derive_shared_secret(const uint8_t* peer_pubkey, uint8_t* out_shared);

// [RCF v1.3] Security & Audit
void rcf_vault_emergency_wipe(void);
void rcf_vault_generate_audit_token(uint16_t session_id);
bool rcf_vault_verify_host_cert(const uint8_t* sig, uint16_t sig_len, 
                                const uint8_t* pubkey, const uint8_t* nonce);

/* [RCF v1.3] Production Key Access (Mil-Spec: OTP) */
const uint8_t* rcf_vault_get_mpk_public(void);
bool           rcf_vault_is_mpk_locked(void);


/* [RCF v1.3] Virtual File System */
void rcf_vault_vfs_store(uint16_t object_id);
void rcf_vault_vfs_fetch(uint16_t object_id);

/* [RCF v1.3] Bridge Key Storage */
void rcf_vault_store_bridge_keys(const uint8_t* enc_key, const uint8_t* mac_key);

/* [RCF v1.3] Sentinel MPK Access */
bool rcf_vault_get_sentinel_mpk(uint8_t* out_mpk, uint32_t len);


// Integrity
bool vault_integrity_check(void);
void vault_zeroize_all(void);


#endif // RCF_VAULT_H