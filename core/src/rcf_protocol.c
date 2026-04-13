/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Core Protocol Implementation for Secure Node Synchronization.
 */

#include "rcf_protocol.h"
#include "rcf_bridge_protocol.h"
#include "rcf_session.h"
#include "rcf_crypto.h"
#include "rcf_vault.h"
#include "rcf_vm.h"
#include "rcf_pilloff.h"
#include "rcf_audit.h"
#include "main.h"
#include <string.h>

/* ─── Internal helpers ───────────────────────────────────────────────────── */

static bool _validate_bridge_magic(uint32_t magic) {
    return (magic == RCF_BRIDGE_MAGIC);
}

/* ─── Command dispatch (Bridge Mode) ─────────────────────────────────────── */

static void _process_bridge_command(RCF_Bridge_Header* header, uint8_t* payload) {
    RCF_Bridge_Header response;
    memset(&response, 0, sizeof(response));
    response.magic = RCF_BRIDGE_MAGIC;
    response.version = RCF_BRIDGE_VERSION;
    response.session_id = header->session_id;

    switch (header->command) {
        case RCF_BCMD_PING:
            response.command = RCF_BCMD_PONG;
            response.result = RCF_BRIDGE_OK;
            /* Send response back to dOS */
            break;

        case RCF_BCMD_VERIFY_PQC: {
            RCF_PQC_VerifyRequest* req = (RCF_PQC_VerifyRequest*)payload;
            /* 
             * [RCF v1.3] Real Dilithium2 verification happens here.
             * Call rcf_pqc_verify(req->signature, req->msg_hash, ...)
             */
            response.command = RCF_BCMD_VERIFY_RESULT;
            response.result = RCF_BRIDGE_OK; // Success for now
            break;
        }

        case RCF_BCMD_PILL_OFF:
            trigger_pill_off(PILL_OFF_IMMEDIATE);
            break;

        default:
            response.command = header->command;
            response.result = RCF_BRIDGE_ERR_BAD_CMD;
            break;
    }
    
    (void)payload;
    /* [STUB] hal_uart_send_bridge(&response, sizeof(response), NULL, 0); */
}

/* ─── Lifecycle ──────────────────────────────────────────────────────────── */

RCF_Error protocol_init(void) {
    session_init();
    return RCF_OK;
}

void protocol_process(void) {
    if (session_is_expired()) {
        session_reset();
        return;
    }

    /* 
     * [RCF v1.3] Bridge Protocol Sniffer.
     * Receives 16-byte frames from ARM64 dOS.
     */
    RCF_Bridge_Header b_header;
    memset(&b_header, 0, sizeof(b_header));

    #ifdef RCF_VM_CI_MODE
    /* In CI, we simulate a PING from dOS */
    b_header.magic = RCF_BRIDGE_MAGIC;
    b_header.command = RCF_BCMD_PING;
    #else
    /* hal_uart_receive(&b_header, sizeof(b_header)); */
    #endif

    if (_validate_bridge_magic(b_header.magic)) {
        _process_bridge_command(&b_header, NULL);
    }
}