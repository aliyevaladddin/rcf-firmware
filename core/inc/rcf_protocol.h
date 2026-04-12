/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * RCF Protocol definitions and packet exchange formats.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#ifndef RCF_PROTOCOL_H
#define RCF_PROTOCOL_H

#include "rcf_config.h"
#include "rcf_session.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* [RCF:PUBLIC] — Protocol error codes */
typedef enum {
    RCF_OK                  = 0,
    RCF_ERR_NULL_PTR        = -1,
    RCF_ERR_INVALID_PARAM   = -2,
    RCF_ERR_CRYPTO_FAIL     = -3,
    RCF_ERR_AUTH_FAIL       = -4,
    RCF_ERR_TIMEOUT         = -5,
    RCF_ERR_TAMPER          = -6,   /* Physical security violation */
    RCF_ERR_PILL_OFF        = -7,   /* System zeroized */
    RCF_ERR_NO_SESSION      = -8,
} RCF_Error;

/* ─── Version ────────────────────────────────────────────────────────────── */

#define RCF_PROTOCOL_VERSION_MAJOR  1
#define RCF_PROTOCOL_VERSION_MINOR  3

/* ─── Magic ──────────────────────────────────────────────────────────────── */

#define RCF_MAGIC_V13   "RCF3"
#define RCF_MAGIC_LEN   4

/* ─── Commands ───────────────────────────────────────────────────────────── */

#define RCF_CMD_HELLO           0x01U
#define RCF_CMD_CHALLENGE       0x02U
#define RCF_CMD_RESPONSE        0x03U
#define RCF_CMD_SESSION         0x04U
#define RCF_CMD_DATA            0x05U
#define RCF_CMD_AUDIT           0x06U
#define RCF_CMD_AUDIT_RESPONSE  0x07U
#define RCF_CMD_PING            0x08U
#define RCF_CMD_EXECUTE_ACODE   0x10U
#define RCF_CMD_ACODE_RESULT    0x11U
#define RCF_CMD_CLOSE_SESSION   0xFEU
#define RCF_CMD_PILL_OFF        0xFFU

/* ─── Markers ────────────────────────────────────────────────────────────── */

#define RCF_MARKER_PUBLIC       0x01U
#define RCF_MARKER_PROTECTED    0x02U
#define RCF_MARKER_RESTRICTED   0x03U

/* ─── v1.3 Hardened Packet Header (Mil-Spec) ─────────────────────────── */

typedef struct __attribute__((packed, aligned(32))) {
    uint8_t  magic[4];          /* "RCF3" */
    uint16_t version;           /* 0x0103 */
    uint8_t  cmd;               /* Command code */
    uint8_t  tier_marker;       /* PUBLIC/PROTECTED/RESTRICTED */
    uint16_t payload_len;       /* Length of encrypted payload */
    uint16_t session_id;        /* Active session identifier */
    uint8_t  reserved[14];      /* Padding for 32-byte alignment */
    uint8_t  gcm_tag[16];       /* Auth tag for payload */
    uint8_t  header_mac[16];    /* HMAC-SHA256 of the header metadata */
} RCF_Packet_Header;

_Static_assert(sizeof(RCF_Packet_Header) == 64, 
               "RCF_Packet_Header must be exactly 64 bytes for Mil-Spec alignment");

/* ─── Public API ─────────────────────────────────────────────────────────── */

RCF_Error protocol_init(void);
void      protocol_process(void);
RCF_Error protocol_establish_session(void);
RCF_Error protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker);
RCF_Error protocol_receive_data(uint8_t* out_buf, uint16_t max_len,
                                uint16_t* out_received, uint8_t* out_marker);

#ifdef __cplusplus
}
#endif

#endif /* RCF_PROTOCOL_H */