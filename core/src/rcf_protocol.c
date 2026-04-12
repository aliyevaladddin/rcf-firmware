/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Core Protocol Implementation for Secure Node Synchronization.
 *
 * VISIBILITY: Manual audit for security purposes is permitted.
 * USAGE: Replication or automated extraction of this logic is strictly
 * prohibited without explicit authorization under RCF-PL v1.3.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#include "rcf_protocol.h"
#include "rcf_crypto.h"
#include "rcf_vault.h"
#include "rcf_session.h"
#include "rcf_vm.h"
#include "usbd_cdc_if.h"

#include <string.h>

/* ─── Module state ───────────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
static RCF_Session_State session;
static bool              session_active = false;

/* ─── Internal helpers ───────────────────────────────────────────────────── */

static bool _validate_magic(const uint8_t magic[RCF_MAGIC_LEN])
{
    return (magic[0] == RCF_MAGIC_0 &&
            magic[1] == RCF_MAGIC_1 &&
            magic[2] == RCF_MAGIC_2 &&
            magic[3] == RCF_MAGIC_3);
}

static void _zero_session(void)
{
    /* Zeroize key material before releasing */
    memset(session.enc_key, 0, RCF_SESSION_KEY_LEN);
    memset(session.mac_key, 0, RCF_SESSION_KEY_LEN);
    memset(&session, 0, sizeof(session));
    session_active = false;
}

static bool _generate_nonce(uint8_t* nonce, size_t len)
{
    /* Use hardware RNG in 4-byte chunks */
    for (size_t i = 0; i < len; i += 4) {
        uint32_t rnd;
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {
            return false;
        }
        size_t copy = (len - i < 4) ? (len - i) : 4;
        memcpy(nonce + i, &rnd, copy);
    }
    return true;
}

/* ─── Lifecycle ──────────────────────────────────────────────────────────── */

RCF_Error protocol_init(void)
{
    _zero_session();
    return RCF_OK;
}

void protocol_reset(void)
{
    _zero_session();
}

/* ─── Session establishment ──────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
RCF_Error protocol_establish_session(void)
{
    /* ── Step 1: Receive HELLO ─────────────────────────────────────────── */
    RCF_Packet_Header hello;
    if (!usb_receive((uint8_t*)&hello, sizeof(hello))) {
        return RCF_ERR_TIMEOUT;
    }
    if (!_validate_magic(hello.magic)) {
        return RCF_ERR_INVALID_MAGIC;
    }
    if (hello.command != RCF_CMD_HELLO) {
        return RCF_ERR_UNKNOWN_CMD;
    }
    if (hello.version != RCF_PROTOCOL_VERSION) {
        return RCF_ERR_VERSION_MISMATCH;
    }

    /* ── Step 2: Generate ephemeral keypair (Curve25519) ──────────────── */
    uint8_t eph_priv[RCF_EPH_KEY_LEN];
    uint8_t eph_pub[RCF_EPH_KEY_LEN];
    curve25519_keygen(eph_pub, eph_priv);

    /* ── Step 3: Generate nonce ───────────────────────────────────────── */
    uint8_t nonce[RCF_NONCE_LEN];
    if (!_generate_nonce(nonce, RCF_NONCE_LEN)) {
        memset(eph_priv, 0, sizeof(eph_priv));
        return RCF_ERR_CRYPTO_FAIL;
    }

    /* ── Step 4: Send CHALLENGE (eph_pub + nonce) ─────────────────────── */
    RCF_Packet_Header challenge;
    memset(&challenge, 0, sizeof(challenge));
    challenge.magic[0]    = RCF_MAGIC_0;
    challenge.magic[1]    = RCF_MAGIC_1;
    challenge.magic[2]    = RCF_MAGIC_2;
    challenge.magic[3]    = RCF_MAGIC_3;
    challenge.version     = RCF_PROTOCOL_VERSION;
    challenge.command     = RCF_CMD_CHALLENGE;
    challenge.payload_len = RCF_EPH_KEY_LEN + RCF_NONCE_LEN;
    challenge.marker      = RCF_MARKER_PUBLIC;
    challenge.session_id  = 0;

    usb_send((uint8_t*)&challenge, sizeof(challenge));
    usb_send(eph_pub, RCF_EPH_KEY_LEN);
    usb_send(nonce,   RCF_NONCE_LEN);

    /* ── Step 5: Receive host RESPONSE (host_eph_pub + host_sig) ─────── */
    uint8_t host_eph_pub[RCF_EPH_KEY_LEN];
    uint8_t host_sig[64];

    if (!usb_receive(host_eph_pub, RCF_EPH_KEY_LEN) ||
        !usb_receive(host_sig, sizeof(host_sig))) {
        memset(eph_priv, 0, sizeof(eph_priv));
        return RCF_ERR_TIMEOUT;
    }

    /* ── Step 6: ECDH — compute shared secret ─────────────────────────── */
    uint8_t shared_secret[RCF_EPH_KEY_LEN];
    curve25519_scalar_mult(shared_secret, eph_priv, host_eph_pub);

    /* Zeroize ephemeral private key immediately after use */
    memset(eph_priv, 0, sizeof(eph_priv));

    /* ── Step 7: Derive session keys via HKDF-SHA256 ──────────────────── */
    const uint8_t info[] = "RCF-v1.3-Session";
    hkdf_sha256(
        shared_secret, sizeof(shared_secret),
        (const uint8_t*)info, sizeof(info) - 1,
        session.enc_key, RCF_SESSION_KEY_LEN,
        session.mac_key, RCF_SESSION_KEY_LEN
    );

    /* Zeroize shared secret after key derivation */
    memset(shared_secret, 0, sizeof(shared_secret));

    /* ── Step 8: Enterprise host authentication ───────────────────────── */
    if (license_check_feature(RCF_FEATURE_ENTERPRISE)) {
        if (!rcf_vault_verify_host_cert(host_sig, sizeof(host_sig),
                                        host_eph_pub, nonce)) {
            _zero_session();
            return RCF_ERR_AUTH_FAILED;
        }
    }

    /* ── Step 9: Finalize session ─────────────────────────────────────── */
    session.session_id          = generate_session_id();
    session.established_time    = get_secure_time();
    session.last_activity_time  = session.established_time;
    session.rx_packet_count     = 0;
    session.tx_packet_count     = 0;
    session_active              = true;

    /* ── Step 10: Send SESSION confirmation (encrypted) ───────────────── */
    uint8_t confirm_payload[2];
    confirm_payload[0] = (session.session_id >> 8) & 0xFF;
    confirm_payload[1] =  session.session_id       & 0xFF;

    return protocol_send_data(confirm_payload, sizeof(confirm_payload),
                              RCF_MARKER_PROTECTED);
}

/* ─── Session teardown ───────────────────────────────────────────────────── */

RCF_Error protocol_close_session(void)
{
    if (!session_active) return RCF_ERR_NO_SESSION;

    uint8_t bye = 0x00;
    protocol_send_data(&bye, 1, RCF_MARKER_PUBLIC);

    _zero_session();
    return RCF_OK;
}

bool protocol_is_session_active(void)
{
    return session_active;
}

bool protocol_is_session_expired(void)
{
    if (!session_active) return false;
    uint32_t now = get_secure_time();
    return (now - session.last_activity_time) > (RCF_SESSION_TIMEOUT_MS / 1000U);
}

/* ─── Command dispatch ───────────────────────────────────────────────────── */

/* [RCF:PROTECTED] */
void protocol_process_command(RCF_Packet_Header* header, uint8_t* payload)
{
    if (!session_active) return;

    session.last_activity_time = get_secure_time();
    session.rx_packet_count++;

    switch (header->command) {

        case RCF_CMD_EXECUTE_ACODE:
            if (header->marker == RCF_MARKER_RESTRICTED) {
                if (!license_check_feature(RCF_FEATURE_ENTERPRISE)) break;
                rcf_vm_execute("AuroraRemoteModule", payload, header->payload_len);
            }
            break;

        case RCF_CMD_AUDIT:
            /* Generate and return signed audit token */
            rcf_vault_generate_audit_token(session.session_id);
            break;

        case RCF_CMD_PING: {
            uint8_t pong = 0x01;
            protocol_send_data(&pong, 1, RCF_MARKER_PUBLIC);
            break;
        }

        case RCF_CMD_CLOSE_SESSION:
            protocol_close_session();
            break;

        case RCF_CMD_PILL_OFF:
            /* Emergency shutdown — zeroize all state immediately */
            _zero_session();
            rcf_vault_emergency_wipe();
            NVIC_SystemReset();
            break;

        default:
            /* Unknown command — log and ignore; do not crash */
            break;
    }
}

void protocol_process(void)
{
    if (protocol_is_session_expired()) {
        protocol_close_session();
        return;
    }

    RCF_Packet_Header header;
    if (!usb_receive((uint8_t*)&header, sizeof(header))) return;
    if (!_validate_magic(header.magic)) return;
    if (header.payload_len > RCF_MAX_PAYLOAD_LEN) return;

    uint8_t payload[RCF_MAX_PAYLOAD_LEN];
    if (header.payload_len > 0) {
        if (!usb_receive(payload, header.payload_len)) return;
    }

    protocol_process_command(&header, payload);
}

/* ─── Data transmission ──────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
RCF_Error protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker)
{
    if (!session_active) return RCF_ERR_NO_SESSION;
    if (len > RCF_MAX_PAYLOAD_LEN) return RCF_ERR_PAYLOAD_TOO_BIG;

    /* License check for restricted marker */
    if (marker == RCF_MARKER_RESTRICTED &&
        !license_check_feature(RCF_FEATURE_ENTERPRISE)) {
        return RCF_ERR_LICENSE_DENIED;
    }

    uint8_t  encrypted[RCF_MAX_PAYLOAD_LEN + RCF_GCM_TAG_LEN];
    uint16_t encrypted_len = 0;

    switch (marker) {
        case RCF_MARKER_PUBLIC:
            memcpy(encrypted, data, len);
            encrypted_len = len;
            break;

        case RCF_MARKER_PROTECTED:
        case RCF_MARKER_RESTRICTED:
            if (!aes256_gcm_encrypt(session.enc_key, data, len,
                                    encrypted, &encrypted_len)) {
                return RCF_ERR_CRYPTO_FAIL;
            }
            break;

        default:
            return RCF_ERR_UNKNOWN_CMD;
    }

    /* Build header */
    RCF_Packet_Header header;
    memset(&header, 0, sizeof(header));
    header.magic[0]    = RCF_MAGIC_0;
    header.magic[1]    = RCF_MAGIC_1;
    header.magic[2]    = RCF_MAGIC_2;
    header.magic[3]    = RCF_MAGIC_3;
    header.version     = RCF_PROTOCOL_VERSION;
    header.command     = RCF_CMD_DATA;
    header.payload_len = encrypted_len;
    header.marker      = marker;
    header.session_id  = session.session_id;

    /* Authenticate header (GCM tag over first 16 bytes of header) */
    gcm_compute_tag(session.mac_key,
                    (uint8_t*)&header,
                    sizeof(header) - RCF_GCM_TAG_LEN,
                    header.auth_tag);

    usb_send((uint8_t*)&header, sizeof(header));
    usb_send(encrypted, encrypted_len);

    session.tx_packet_count++;
    session.last_activity_time = get_secure_time();

    return RCF_OK;
}

RCF_Error protocol_receive_data(uint8_t* out_buf, uint16_t max_len,
                                uint16_t* out_received, uint8_t* out_marker)
{
    if (!session_active) return RCF_ERR_NO_SESSION;

    RCF_Packet_Header header;
    if (!usb_receive((uint8_t*)&header, sizeof(header))) return RCF_ERR_TIMEOUT;
    if (!_validate_magic(header.magic)) return RCF_ERR_INVALID_MAGIC;
    if (header.command != RCF_CMD_DATA) return RCF_ERR_UNKNOWN_CMD;
    if (header.payload_len > max_len) return RCF_ERR_PAYLOAD_TOO_BIG;

    uint8_t raw[RCF_MAX_PAYLOAD_LEN];
    if (!usb_receive(raw, header.payload_len)) return RCF_ERR_TIMEOUT;

    if (header.marker == RCF_MARKER_PUBLIC) {
        memcpy(out_buf, raw, header.payload_len);
        *out_received = header.payload_len;
    } else {
        uint16_t decrypted_len = 0;
        if (!aes256_gcm_decrypt(session.enc_key, raw, header.payload_len,
                                out_buf, &decrypted_len)) {
            return RCF_ERR_CRYPTO_FAIL;
        }
        *out_received = decrypted_len;
    }

    if (out_marker) *out_marker = header.marker;

    session.rx_packet_count++;
    session.last_activity_time = get_secure_time();

    return RCF_OK;
}

/* ─── Diagnostics ────────────────────────────────────────────────────────── */

uint32_t protocol_get_rx_count(void) { return session.rx_packet_count; }
uint32_t protocol_get_tx_count(void) { return session.tx_packet_count; }