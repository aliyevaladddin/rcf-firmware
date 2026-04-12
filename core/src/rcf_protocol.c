/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Core Protocol Implementation for Secure Node Synchronization.
 */

#include "rcf_protocol.h"
#include "rcf_session.h"
#include "rcf_crypto.h"
#include "rcf_vault.h"
#include "rcf_vm.h"
#include "rcf_pilloff.h"
#include "rcf_audit.h"
#include "main.h"
#include <string.h>

/* ─── Internal helpers ───────────────────────────────────────────────────── */

static bool _validate_magic(const uint8_t magic[RCF_MAGIC_LEN]) {
    return (memcmp(magic, RCF_MAGIC_V13, RCF_MAGIC_LEN) == 0);
}

/* ─── Lifecycle ──────────────────────────────────────────────────────────── */

RCF_Error protocol_init(void) {
    session_init();
    return RCF_OK;
}

/* ─── Session establishment ──────────────────────────────────────────────── */

RCF_Error protocol_establish_session(void) {
    /* [RCF v1.3] Delegated to session module for hardening */
    if (session_establish()) {
        return RCF_OK;
    }
    return RCF_ERR_AUTH_FAIL;
}

/* ─── Command dispatch ───────────────────────────────────────────────────── */

static void _process_command(RCF_Packet_Header* header, uint8_t* payload) {
    if (!session_is_active()) return;

    session_update_activity();

    switch (header->cmd) {
        case RCF_CMD_PING: {
            uint8_t pong = 0x01;
            protocol_send_data(&pong, 1, RCF_MARKER_PUBLIC);
            break;
        }

        case RCF_CMD_CLOSE_SESSION:
            session_reset();
            break;

        case RCF_CMD_PILL_OFF:
            trigger_pill_off(PILL_OFF_IMMEDIATE);
            break;

        default:
            break;
    }
    (void)payload;
}

void protocol_process(void) {
    if (session_is_expired()) {
        session_reset();
        return;
    }

    RCF_Packet_Header header;
    /* In CI mode, we mock the reception */
    #ifdef RCF_VM_CI_MODE
    memset(&header, 0, sizeof(header));
    #else
    // Actual USB receive logic here
    #endif

    if (_validate_magic(header.magic)) {
        _process_command(&header, NULL);
    }
}

/* ─── Data transmission ──────────────────────────────────────────────────── */

RCF_Error protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker) {
    if (!session_is_active()) return RCF_ERR_NO_SESSION;

    RCF_Packet_Header header;
    memset(&header, 0, sizeof(header));
    memcpy(header.magic, RCF_MAGIC_V13, RCF_MAGIC_LEN);
    header.cmd = RCF_CMD_DATA;
    header.payload_len = len;
    header.tier_marker = marker;
    header.session_id = session_get_id();

    const uint8_t* enc_key = session_get_enc_key();
    
    if (marker != RCF_MARKER_PUBLIC) {
        /* [RCF v1.3] AES-GCM Encryption logic using enc_key */
        uint8_t tag[16];
        uint8_t ciphertext[len];
        rcf_aes256_gcm_encrypt(enc_key, NULL, data, len, ciphertext, tag);
        memcpy(header.gcm_tag, tag, 16);
    }

    /* Compute HMAC over header for integrity */
    const uint8_t* mac_key = session_get_mac_key();
    rcf_hmac_sha256(mac_key, 32, (uint8_t*)&header, sizeof(header) - 16, header.header_mac);

    /* [STUB] usb_send(&header, sizeof(header)); usb_send(data, len); */
    session_update_activity();

    return RCF_OK;
}

RCF_Error protocol_receive_data(uint8_t* out_buf, uint16_t max_len,
                                uint16_t* out_received, uint8_t* out_marker) {
    if (!session_is_active()) return RCF_ERR_NO_SESSION;

    /* [RCF v1.3] Receive and decrypt session-bound data */
    (void)out_buf; (void)max_len; (void)out_received; (void)out_marker;
    return RCF_OK; // Stub
}