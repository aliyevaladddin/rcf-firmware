/*
 * [RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 *
 * rcf_bridge_protocol.h — Shared protocol definitions for
 * ARM64 (dOS) <-> STM32 (HSM) integration.
 *
 * This header is the SINGLE SOURCE OF TRUTH for the bridge protocol.
 * Include on both sides. Do not duplicate definitions.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#ifndef RCF_BRIDGE_PROTOCOL_H
#define RCF_BRIDGE_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Version ────────────────────────────────────────────────────────────── */

#define RCF_BRIDGE_VERSION          0x0103U   /* v1.3 */
#define RCF_BRIDGE_MAGIC            0x52434642U /* "RCFB" little-endian */

/* ─── Bridge commands ────────────────────────────────────────────────────── */

/*
 * Handshake & session
 */
#define RCF_BCMD_HELLO              0x01U   /* ARM64 → STM32: initiate      */
#define RCF_BCMD_CHALLENGE          0x02U   /* STM32 → ARM64: eph_pub+nonce */
#define RCF_BCMD_RESPONSE           0x03U   /* ARM64 → STM32: eph_pub+sig   */
#define RCF_BCMD_SESSION_OK         0x04U   /* STM32 → ARM64: session ready */

/*
 * PQC delegation — ARM64 delegates verification to STM32 Bunker
 */
#define RCF_BCMD_VERIFY_PQC         0x10U   /* ARM64 → STM32: verify sig    */
#define RCF_BCMD_VERIFY_RESULT      0x11U   /* STM32 → ARM64: PASS / FAIL   */

/*
 * A-VM remote execution
 */
#define RCF_BCMD_EXEC_ACODE         0x20U   /* ARM64 → STM32: run .acode    */
#define RCF_BCMD_EXEC_RESULT        0x21U   /* STM32 → ARM64: VM_Result     */

/*
 * Key operations (delegated to HSM)
 */
#define RCF_BCMD_KEYGEN_REQ         0x30U   /* ARM64 → STM32: generate key  */
#define RCF_BCMD_KEYGEN_PUBKEY      0x31U   /* STM32 → ARM64: public key    */
#define RCF_BCMD_SIGN_REQ           0x32U   /* ARM64 → STM32: sign payload  */
#define RCF_BCMD_SIGN_RESULT        0x33U   /* STM32 → ARM64: signature     */

/*
 * Heartbeat & diagnostics
 */
#define RCF_BCMD_PING               0x40U   /* ARM64 → STM32: keepalive     */
#define RCF_BCMD_PONG               0x41U   /* STM32 → ARM64: alive         */
#define RCF_BCMD_GET_STATUS         0x42U   /* ARM64 → STM32: HSM status    */
#define RCF_BCMD_STATUS_REPORT      0x43U   /* STM32 → ARM64: status data   */

/*
 * Emergency
 */
#define RCF_BCMD_PILL_OFF           0xFFU   /* Either side: emergency halt  */

/* ─── Result codes ───────────────────────────────────────────────────────── */

#define RCF_BRIDGE_OK               0x00U
#define RCF_BRIDGE_ERR_SIG_FAIL     0x01U
#define RCF_BRIDGE_ERR_NO_SESSION   0x02U
#define RCF_BRIDGE_ERR_LICENSE      0x03U
#define RCF_BRIDGE_ERR_VM_FAIL      0x04U
#define RCF_BRIDGE_ERR_TIMEOUT      0x05U
#define RCF_BRIDGE_ERR_BAD_CMD      0x06U

/* ─── Sizes ──────────────────────────────────────────────────────────────── */

#define RCF_BRIDGE_DILITHIUM2_SIG   2420U
#define RCF_BRIDGE_EPH_KEY_LEN      32U
#define RCF_BRIDGE_NONCE_LEN        16U
#define RCF_BRIDGE_SESSION_KEY_LEN  32U
#define RCF_BRIDGE_MAX_PAYLOAD      4096U
#define RCF_BRIDGE_HEADER_SIZE      16U

/* ─── Frame header ───────────────────────────────────────────────────────── */

/*
 * Wire format (packed, little-endian):
 *
 *   Offset  Size  Field
 *   0       4     magic        0x52434642 "RCFB"
 *   4       2     version      0x0103
 *   6       1     command      RCF_BCMD_*
 *   7       1     result       RCF_BRIDGE_OK / error (in response frames)
 *   8       2     payload_len  bytes following header
 *   10      2     session_id   0 before session established
 *   12      4     crc32        CRC-32 over header[0:12] + payload
 *   --- Total: 16 bytes ---
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;          /* RCF_BRIDGE_MAGIC                     */
    uint16_t version;        /* RCF_BRIDGE_VERSION                   */
    uint8_t  command;        /* RCF_BCMD_*                           */
    uint8_t  result;         /* RCF_BRIDGE_OK or error code          */
    uint16_t payload_len;    /* Payload bytes following this header  */
    uint16_t session_id;     /* 0 before session                     */
    uint32_t crc32;          /* CRC-32 of header[0:12] + payload     */
} RCF_Bridge_Header;

_Static_assert(sizeof(RCF_Bridge_Header) == RCF_BRIDGE_HEADER_SIZE,
               "RCF_Bridge_Header must be exactly 16 bytes");

/* ─── PQC verification request payload ──────────────────────────────────── */

/*
 * Sent by ARM64 with RCF_BCMD_VERIFY_PQC.
 * STM32 verifies: Dilithium2_Verify(sig, msg_hash, SENTINEL_MPK)
 */
typedef struct __attribute__((packed)) {
    uint8_t  signature[RCF_BRIDGE_DILITHIUM2_SIG];  /* Dilithium2 sig      */
    uint8_t  msg_hash[32];                           /* SHA-256 of .text    */
    uint32_t msg_len;                                /* Original message len*/
} RCF_PQC_VerifyRequest;

/* ─── HSM status report payload ─────────────────────────────────────────── */

typedef struct __attribute__((packed)) {
    uint8_t  rcf_version[2];       /* {1, 3}                               */
    uint8_t  bunker_active;        /* 1 if Bunker currently engaged        */
    uint8_t  timechain_valid;      /* 1 if Timechain integrity OK          */
    uint32_t monotonic_counter;    /* Current Timechain monotonic value    */
    uint16_t voltage_mv;           /* VBAT voltage                         */
    int16_t  temperature_celsius;  /* STM32 internal sensor                */
    uint32_t session_rx_count;     /* Packets received this session        */
    uint32_t session_tx_count;     /* Packets sent this session            */
} RCF_HSM_Status;

#ifdef __cplusplus
}
#endif

#endif /* RCF_BRIDGE_PROTOCOL_H */