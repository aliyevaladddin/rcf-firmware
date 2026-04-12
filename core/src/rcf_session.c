/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Session Management — Lifecycle & Security Implementation.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "rcf_session.h"
#include "rcf_crypto.h"
#include "rcf_vault.h"
#include "rcf_audit.h"
#include "main.h"  /* For HAL access */
#include <string.h>

/* [RCF:RESTRICTED] Internal module state */
static RCF_Session_State current_session;
static bool              is_active = false;

/* ─── Internal Helpers ───────────────────────────────────────────────────── */

static void _zeroize(void) {
    memset(current_session.enc_key, 0, RCF_SESSION_KEY_LEN);
    memset(current_session.mac_key, 0, RCF_SESSION_KEY_LEN);
    memset(&current_session, 0, sizeof(current_session));
    is_active = false;
}

static bool _generate_nonce(uint8_t* out, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t rnd;
        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
        size_t copy = (len - i < 4) ? (len - i) : 4;
        memcpy(out + i, &rnd, copy);
    }
    return true;
}

/* [RCF v1.3] Simple counter for session IDs */
static uint16_t _next_id(void) {
    static uint16_t id = 0x1000;
    return id++;
}

/* ─── Lifecycle API ──────────────────────────────────────────────────────── */

void session_init(void) {
    _zeroize();
}

void session_reset(void) {
    _zeroize();
}

bool session_is_active(void) {
    return is_active;
}

bool session_is_expired(void) {
    if (!is_active) return false;
    uint32_t now = HAL_GetTick() / 1000;
    return (now - current_session.last_activity_time) > RCF_SESSION_TIMEOUT_S;
}

uint16_t session_get_id(void) {
    return current_session.session_id;
}

void session_update_activity(void) {
    current_session.last_activity_time = HAL_GetTick() / 1000;
}

const uint8_t* session_get_enc_key(void) {
    return current_session.enc_key;
}

const uint8_t* session_get_mac_key(void) {
    return current_session.mac_key;
}

/* ─── Security Handshake ─────────────────────────────────────────────────── */

bool session_establish(void) {
    uint8_t eph_priv[32];
    uint8_t eph_pub[32];
    uint8_t shared_secret[32];
    
    /* 1. Reset any existing session */
    _zeroize();

    /* 2. Generate Ephemeral Keypair (Curve25519) */
    rcf_curve25519_keygen(eph_pub, eph_priv);

    /* 3. Handshake Mockup (In reality, this would exchange packets) */
    /* This stub assumes the host pubkey is known or received via protocol.c */
    uint8_t host_eph_pub[32];
    _generate_nonce(host_eph_pub, 32); // [STUB]

    /* 4. Derive Shared Secret */
    rcf_curve25519_shared(eph_priv, host_eph_pub, shared_secret);
    
    /* 5. Derive Session Keys (KDF) — Use SHA256 as placeholder for HKDF */
    rcf_hmac_sha256(shared_secret, 32, (uint8_t*)"ENC", 3, current_session.enc_key);
    rcf_hmac_sha256(shared_secret, 32, (uint8_t*)"MAC", 3, current_session.mac_key);

    /* 6. Cleanup sensitive buffers */
    memset(eph_priv, 0, sizeof(eph_priv));
    memset(shared_secret, 0, sizeof(shared_secret));

    /* 7. Finalize State */
    current_session.session_id = _next_id();
    current_session.established_time = HAL_GetTick() / 1000;
    current_session.last_activity_time = current_session.established_time;
    is_active = true;

    return true;
}
