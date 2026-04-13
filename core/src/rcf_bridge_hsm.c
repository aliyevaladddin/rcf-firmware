/*
 * [RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 *
 * rcf_bridge_hsm.c — STM32 (HSM) side bridge handler.
 */

#include "rcf_bridge_protocol.h"
#include "rcf_protocol.h"
#include "rcf_vm.h"
#include "rcf_bunker.h"
#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_timechain.h"
#include "rcf_pilloff.h"
#include "rcf_audit.h"
#include "rcf_bridge_hsm.h"
#include "stm32f4xx_hal.h"

#include <string.h>

/* ─── External dependencies ──────────────────────────────────────────────── */

extern RNG_HandleTypeDef hrng;

/* Forward declarations for internal HAL stubs */
void hal_uart_send(const uint8_t* data, size_t len);
bool hal_uart_receive(uint8_t* data, size_t len);

/* ─── Internal state ─────────────────────────────────────────────────────── */

static bool     hsm_bridge_active = false;
static uint16_t hsm_session_id    = 0;

/* ─── CRC-32 (same polynomial as ARM64 side) ─────────────────────────────── */

static uint32_t _crc32(const uint8_t* data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint32_t)data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320U & -(crc & 1U));
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

/* ─── Transport ──────────────────────────────────────────────────────────── */

#ifdef RCF_VM_CI_MODE
#define BRIDGE_RECV(ptr, len)  hal_uart_receive(ptr, len)
#define BRIDGE_SEND(ptr, len)  hal_uart_send(ptr, len)
#else
/* Production: USB CDC */
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
#define BRIDGE_RECV(ptr, len)  false /* USB RX is interrupt-driven */
#define BRIDGE_SEND(ptr, len)  CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len)
#endif

static bool _recv_frame(RCF_Bridge_Header* hdr,
                        uint8_t* payload, uint16_t max_payload)
{
    if (!BRIDGE_RECV((uint8_t*)hdr, sizeof(*hdr))) return false;
    if (hdr->magic   != RCF_BRIDGE_MAGIC)   return false;
    if (hdr->version != RCF_BRIDGE_VERSION) return false;
    if (hdr->payload_len > max_payload)      return false;

    if (hdr->payload_len > 0) {
        if (!BRIDGE_RECV(payload, hdr->payload_len)) return false;
    }

    uint32_t expected = _crc32((const uint8_t*)hdr, 12);
    if (hdr->payload_len > 0) {
        expected ^= _crc32(payload, hdr->payload_len);
    }
    if (expected != hdr->crc32) return false;

    return true;
}

static void _send_response(uint8_t cmd, uint8_t result,
                           const uint8_t* payload, uint16_t payload_len)
{
    RCF_Bridge_Header resp;
    resp.magic       = RCF_BRIDGE_MAGIC;
    resp.version     = RCF_BRIDGE_VERSION;
    resp.command     = cmd;
    resp.result      = result;
    resp.payload_len = payload_len;
    resp.session_id  = hsm_session_id;

    uint32_t crc = _crc32((const uint8_t*)&resp, 12);
    if (payload && payload_len > 0) {
        crc ^= _crc32(payload, payload_len);
    }
    resp.crc32 = crc;

    BRIDGE_SEND((const uint8_t*)&resp, sizeof(resp));
    if (payload && payload_len > 0) {
        BRIDGE_SEND(payload, payload_len);
    }
}

/* ─── Command handlers ───────────────────────────────────────────────────── */

static void _handle_hello(void)
{
    uint8_t eph_priv[32], eph_pub[32];
    rcf_curve25519_keygen(eph_pub, eph_priv);

    uint8_t nonce[RCF_BRIDGE_NONCE_LEN];
    for (int i = 0; i < 4; i++) {
        uint32_t rnd;
        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
        memcpy(nonce + i * 4, &rnd, 4);
    }

    rcf_bunker_store_ephemeral(eph_priv, 32);
    memset(eph_priv, 0, sizeof(eph_priv));

    uint8_t challenge[RCF_BRIDGE_EPH_KEY_LEN + RCF_BRIDGE_NONCE_LEN];
    memcpy(challenge, eph_pub, RCF_BRIDGE_EPH_KEY_LEN);
    memcpy(challenge + RCF_BRIDGE_EPH_KEY_LEN, nonce, RCF_BRIDGE_NONCE_LEN);

    _send_response(RCF_BCMD_CHALLENGE, RCF_BRIDGE_OK,
                   challenge, sizeof(challenge));
}

static void _handle_response(const uint8_t* payload, uint16_t len)
{
    if (len < RCF_BRIDGE_EPH_KEY_LEN + 64) {
        _send_response(RCF_BCMD_SESSION_OK, RCF_BRIDGE_ERR_BAD_CMD, NULL, 0);
        return;
    }

    const uint8_t* arm_eph_pub = payload;
    uint8_t eph_priv[32];
    rcf_bunker_retrieve_ephemeral(eph_priv, 32);

    uint8_t shared[32];
    rcf_curve25519_shared(eph_priv, arm_eph_pub, shared);
    memset(eph_priv, 0, sizeof(eph_priv));

    uint8_t enc_key[32], mac_key[32];
    const uint8_t info[] = "RCF-v1.3-Bridge";
    rcf_hkdf_sha256(NULL, 0, shared, 32, info, sizeof(info) - 1, enc_key, 32);
    /* In CI we just reuse derived bits for MAC to simplify */
    memcpy(mac_key, enc_key, 32);
    for(int i=0; i<32; i++) mac_key[i] ^= 0xFF;

    memset(shared, 0, sizeof(shared));
    rcf_vault_store_bridge_keys(enc_key, mac_key);
    memset(enc_key, 0, sizeof(enc_key));
    memset(mac_key, 0, sizeof(mac_key));

    uint32_t rnd_id;
    HAL_RNG_GenerateRandomNumber(&hrng, &rnd_id);
    hsm_session_id    = (uint16_t)rnd_id;
    hsm_bridge_active = true;

    rcf_audit_log(EVENT_BRIDGE_SESSION_START, hsm_session_id);

    uint8_t session_payload[2] = {
        (uint8_t)((hsm_session_id >> 8) & 0xFF),
        (uint8_t)(hsm_session_id & 0xFF)
    };
    _send_response(RCF_BCMD_SESSION_OK, RCF_BRIDGE_OK,
                   session_payload, sizeof(session_payload));
}

static void _handle_verify_pqc(const uint8_t* payload, uint16_t len)
{
    if (!hsm_bridge_active) {
        _send_response(RCF_BCMD_VERIFY_RESULT,
                       RCF_BRIDGE_ERR_NO_SESSION, NULL, 0);
        return;
    }

    if (len < sizeof(RCF_PQC_VerifyRequest)) {
        _send_response(RCF_BCMD_VERIFY_RESULT,
                       RCF_BRIDGE_ERR_BAD_CMD, NULL, 0);
        return;
    }

    const RCF_PQC_VerifyRequest* req = (const RCF_PQC_VerifyRequest*)payload;
    rcf_audit_log(EVENT_BRIDGE_PQC_VERIFY_REQ, req->msg_len);

    rcf_bunker_enter();
    uint8_t mpk[1312];
    if (rcf_vault_get_sentinel_mpk(mpk, sizeof(mpk))) {
        int result = rcf_pqc_verify(req->signature, req->msg_hash,
                                    (int)req->msg_len, mpk);
        uint8_t ok = (result == 0) ? RCF_BRIDGE_OK : RCF_BRIDGE_ERR_SIG_FAIL;
        rcf_audit_log(EVENT_BRIDGE_PQC_VERIFY_RESULT, ok);
        _send_response(RCF_BCMD_VERIFY_RESULT, ok, NULL, 0);
    } else {
        _send_response(RCF_BCMD_VERIFY_RESULT, RCF_BRIDGE_ERR_BAD_CMD, NULL, 0);
    }
    
    memset(mpk, 0, sizeof(mpk));
    rcf_bunker_exit();
}

static void _handle_exec_acode(const uint8_t* payload, uint16_t len)
{
    if (!hsm_bridge_active) {
        _send_response(RCF_BCMD_EXEC_RESULT,
                       RCF_BRIDGE_ERR_NO_SESSION, NULL, 0);
        return;
    }
    rcf_audit_log(EVENT_BRIDGE_ACODE_EXEC, len);
    int vm_result = rcf_vm_execute("bridge_remote", (void*)payload, len);
    _send_response(RCF_BCMD_EXEC_RESULT, (uint8_t)(int8_t)vm_result, NULL, 0);
}

static void _handle_get_status(void)
{
    RCF_HSM_Status status;
    memset(&status, 0, sizeof(status));
    status.rcf_version[0]  = 1;
    status.rcf_version[1]  = 3;
    status.bunker_active   = (uint8_t)rcf_vm_in_bunker();
    status.timechain_valid = (uint8_t)timechain_is_valid();
    status.monotonic_counter = timechain_get_monotonic();
    status.voltage_mv        = get_vbat_voltage();
    status.temperature_celsius = get_internal_temperature();
    
    _send_response(RCF_BCMD_STATUS_REPORT, RCF_BRIDGE_OK,
                   (const uint8_t*)&status, sizeof(status));
}

/* ─── Main bridge process loop ───────────────────────────────────────────── */

void rcf_bridge_hsm_process(void)
{
    RCF_Bridge_Header hdr;
    static uint8_t payload[RCF_BRIDGE_MAX_PAYLOAD];

    if (!_recv_frame(&hdr, payload, sizeof(payload))) return;

    switch (hdr.command) {
        case RCF_BCMD_HELLO:
            _handle_hello();
            break;
        case RCF_BCMD_RESPONSE:
            _handle_response(payload, hdr.payload_len);
            break;
        case RCF_BCMD_VERIFY_PQC:
            _handle_verify_pqc(payload, hdr.payload_len);
            break;
        case RCF_BCMD_EXEC_ACODE:
            _handle_exec_acode(payload, hdr.payload_len);
            break;
        case RCF_BCMD_PING:
            _send_response(RCF_BCMD_PONG, RCF_BRIDGE_OK, NULL, 0);
            break;
        case RCF_BCMD_GET_STATUS:
            _handle_get_status();
            break;
        case RCF_BCMD_PILL_OFF:
            rcf_vault_emergency_wipe();
            trigger_pill_off(PILL_OFF_EXTERNAL);
            break;
        default:
            rcf_audit_log(EVENT_BRIDGE_UNKNOWN_CMD, hdr.command);
            _send_response(hdr.command, RCF_BRIDGE_ERR_BAD_CMD, NULL, 0);
            break;
    }
}

bool rcf_bridge_hsm_is_active(void) { return hsm_bridge_active; }