/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * Session Management — Lifecycle & Security.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef RCF_SESSION_H
#define RCF_SESSION_H

#include <stdint.h>
#include <stdbool.h>
#include "rcf_config.h"

#define RCF_SESSION_KEY_LEN     32U
#define RCF_NONCE_LEN           16U
#define RCF_EPH_KEY_LEN         32U
#define RCF_SESSION_TIMEOUT_S   300U

typedef struct {
    uint16_t session_id;
    uint8_t  enc_key[RCF_SESSION_KEY_LEN];
    uint8_t  mac_key[RCF_SESSION_KEY_LEN];
    uint32_t established_time;
    uint32_t last_activity_time;
    uint32_t rx_packet_count;
    uint32_t tx_packet_count;
} RCF_Session_State;

/* ─── Lifecycle API ──────────────────────────────────────────────────────── */

void session_init(void);
void session_reset(void);
bool session_is_active(void);
bool session_is_expired(void);
uint16_t session_get_id(void);
void session_update_activity(void);

/* ─── Security API ───────────────────────────────────────────────────────── */

/* Returns internal key material for AES-GCM operations */
const uint8_t* session_get_enc_key(void);
const uint8_t* session_get_mac_key(void);

/* [RCF:RESTRICTED] Perform ECDH handshake and derive keys */
bool session_establish(void);

#endif /* RCF_SESSION_H */
