/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * RCF Protocol definitions and packet exchange formats.
 *
 * VISIBILITY: Manual audit for security purposes is permitted.
 * USAGE: Replication or automated extraction of this logic is strictly
 * prohibited without explicit authorization under RCF-PL v1.3.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#ifndef RCF_PROTOCOL_H
#define RCF_PROTOCOL_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Version ────────────────────────────────────────────────────────────── */

#define RCF_PROTOCOL_VERSION_MAJOR  1
#define RCF_PROTOCOL_VERSION_MINOR  3
#define RCF_PROTOCOL_VERSION        ((RCF_PROTOCOL_VERSION_MAJOR << 8) | \
                                      RCF_PROTOCOL_VERSION_MINOR)

/* ─── Magic ──────────────────────────────────────────────────────────────── */

#define RCF_MAGIC_0   'R'
#define RCF_MAGIC_1   'C'
#define RCF_MAGIC_2   'F'
#define RCF_MAGIC_3   '\0'
#define RCF_MAGIC_LEN 4

/* ─── Commands ───────────────────────────────────────────────────────────── */

/* Handshake */
#define RCF_CMD_HELLO           0x01U   /* Host → Device: initiate session */
#define RCF_CMD_CHALLENGE       0x02U   /* Device → Host: eph_pub + nonce  */
#define RCF_CMD_RESPONSE        0x03U   /* Host → Device: eph_pub + sig    */
#define RCF_CMD_SESSION         0x04U   /* Device → Host: session_id + enc */

/* Data */
#define RCF_CMD_DATA            0x05U   /* Bidirectional encrypted payload  */
#define RCF_CMD_AUDIT           0x06U   /* Request audit token              */
#define RCF_CMD_AUDIT_RESPONSE  0x07U   /* Return signed audit report       */
#define RCF_CMD_PING            0x08U   /* Keepalive                        */

/* Execution */
#define RCF_CMD_EXECUTE_ACODE   0x10U   /* Execute signed .acode module     */
#define RCF_CMD_ACODE_RESULT    0x11U   /* Return execution result          */

/* Control */
#define RCF_CMD_CLOSE_SESSION   0xFEU   /* Graceful session teardown        */
#define RCF_CMD_PILL_OFF        0xFFU   /* Emergency shutdown               */

/* ─── Markers ────────────────────────────────────────────────────────────── */

#define RCF_MARKER_PUBLIC       0x01U   /* No encryption, framing only      */
#define RCF_MARKER_PROTECTED    0x02U   /* AES-256-GCM encrypted            */
#define RCF_MARKER_RESTRICTED   0x03U   /* AES-256-GCM + enterprise check   */

/* ─── Limits ─────────────────────────────────────────────────────────────── */

#define RCF_MAX_PAYLOAD_LEN     4096U
#define RCF_SESSION_TIMEOUT_MS  30000U
#define RCF_GCM_TAG_LEN         16U
#define RCF_EPH_KEY_LEN         32U
#define RCF_NONCE_LEN           16U
#define RCF_SESSION_KEY_LEN     32U

/* ─── Packet header ──────────────────────────────────────────────────────── */

/*
 * Wire format (little-endian, packed):
 *
 *   Offset  Size  Field
 *   0       4     magic[4]       "RCF\0"
 *   4       2     version        RCF_PROTOCOL_VERSION
 *   6       1     command        RCF_CMD_*
 *   7       2     payload_len    length of payload following the header
 *   9       1     marker         RCF_MARKER_*
 *   10      2     session_id     0x0000 before session established
 *   12      2     reserved       must be 0x0000
 *   14      2     flags          see RCF_FLAG_*
 *   16      16    auth_tag       GCM authentication tag over header[0:16]
 *   --- Total: 32 bytes ---
 */
typedef struct __attribute__((packed)) {
    uint8_t  magic[RCF_MAGIC_LEN];   /* "RCF\0"                          */
    uint16_t version;                 /* RCF_PROTOCOL_VERSION             */
    uint8_t  command;                 /* RCF_CMD_*                        */
    uint16_t payload_len;             /* Payload bytes following header   */
    uint8_t  marker;                  /* RCF_MARKER_*                     */
    uint16_t session_id;              /* Session identifier               */
    uint16_t reserved;                /* Must be zero                     */
    uint16_t flags;                   /* RCF_FLAG_*                       */
    uint8_t  auth_tag[RCF_GCM_TAG_LEN]; /* GCM tag over header[0:16]    */
} RCF_Packet_Header;

/* Compile-time size assertion */
_Static_assert(sizeof(RCF_Packet_Header) == 32,
               "RCF_Packet_Header must be exactly 32 bytes");

/* ─── Flags ──────────────────────────────────────────────────────────────── */

#define RCF_FLAG_NONE           0x0000U
#define RCF_FLAG_COMPRESSED     0x0001U /* Payload is zlib compressed       */
#define RCF_FLAG_FRAGMENTED     0x0002U /* More fragments follow            */
#define RCF_FLAG_LAST_FRAGMENT  0x0004U /* This is the last fragment        */
#define RCF_FLAG_AUDIT_REQ      0x0008U /* Audit trail requested            */

/* ─── Session state ──────────────────────────────────────────────────────── */

typedef struct {
    uint16_t session_id;
    uint8_t  enc_key[RCF_SESSION_KEY_LEN];    /* AES-256-GCM encryption key */
    uint8_t  mac_key[RCF_SESSION_KEY_LEN];    /* HMAC-SHA256 / GCM MAC key  */
    uint32_t established_time;                 /* Secure timestamp (seconds) */
    uint32_t last_activity_time;
    uint32_t rx_packet_count;
    uint32_t tx_packet_count;
} RCF_Session_State;

/* ─── Error codes ────────────────────────────────────────────────────────── */

typedef enum {
    RCF_OK                  =  0,
    RCF_ERR_INVALID_MAGIC   = -1,
    RCF_ERR_VERSION_MISMATCH= -2,
    RCF_ERR_AUTH_FAILED     = -3,
    RCF_ERR_NO_SESSION      = -4,
    RCF_ERR_PAYLOAD_TOO_BIG = -5,
    RCF_ERR_LICENSE_DENIED  = -6,
    RCF_ERR_CRYPTO_FAIL     = -7,
    RCF_ERR_TIMEOUT         = -8,
    RCF_ERR_UNKNOWN_CMD     = -9,
} RCF_Error;

/* ─── Public API ─────────────────────────────────────────────────────────── */

/* Lifecycle */
RCF_Error protocol_init(void);
void      protocol_process(void);
void      protocol_reset(void);

/* Session management */
RCF_Error protocol_establish_session(void);
RCF_Error protocol_close_session(void);
bool      protocol_is_session_active(void);
bool      protocol_is_session_expired(void);

/* Data transmission */
RCF_Error protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker);
RCF_Error protocol_receive_data(uint8_t* out_buf, uint16_t max_len,
                                uint16_t* out_received, uint8_t* out_marker);

/* Diagnostics */
uint32_t  protocol_get_rx_count(void);
uint32_t  protocol_get_tx_count(void);

#ifdef __cplusplus
}
#endif

#endif /* RCF_PROTOCOL_H */