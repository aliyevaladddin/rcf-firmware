/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * Secure Key Vault & Anti-Tamper Storage Definitions.
 */

#ifndef RCF_VAULT_H
#define RCF_VAULT_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VAULT_KEY_DEVICE_PRIV     = 0,   // Curve25519 static key
    VAULT_KEY_LICENSE_HMAC    = 1,   // License validation key
    VAULT_KEY_SESSION_EPHEMERAL = 2, // Ephemeral per-session
    VAULT_KEY_AUDIT_SIGN      = 3,   // Ed25519 audit signing
    VAULT_KEY_COUNT           = 4
} Vault_KeyType;

typedef struct {
    uint8_t key_id;
    uint8_t key_data[64];        // Max size (Ed25519 private)
    uint32_t key_attributes;     // Permissions, lifetime
    uint8_t key_checksum[32];    // SHA-256 for integrity
} Vault_KeyEntry;

// Lifecycle
bool vault_init(void);
bool vault_factory_reset(void);
bool vault_is_initialized(void);

// Key operations
bool vault_load_key(Vault_KeyType type, uint8_t* out_buffer, uint32_t* out_len);
bool vault_generate_ephemeral(uint8_t* out_pubkey, uint8_t* out_privkey);
bool vault_derive_shared_secret(const uint8_t* peer_pubkey, uint8_t* out_shared);

// Security
void vault_zeroize_all(void);
bool vault_integrity_check(void);

#endif // RCF_VAULT_H