/* 
 * [RCF:NOTICE][RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * Core Protocol Implementation for Secure Node Synchronization.
 * 
 * VISIBILITY: Manual audit for security purposes is allowed.
 * USAGE: Replication or automated extraction of this logic is strictly 
 * prohibited without explicit authorization under RCF-PL.
 */

#include "rcf_protocol.h"
#include "rcf_crypto.h"
#include "rcf_vault.h"
#include "rcf_session.h"
#include "rcf_vm.h"
#include "usbd_cdc_if.h"

static RCF_Session_State session;
static bool session_active = false;

bool protocol_establish_session(void) {
    // [RCF-START][M-SYNC-ESTABLISH]
    // 1. Receive HELLO from host
    RCF_Packet_Header hello;
    if (!usb_receive((uint8_t*)&hello, sizeof(hello))) return false;
    
    if (memcmp(hello.magic, "RCF", 3) != 0) return false;
    if (hello.command != RCF_CMD_HELLO) return false;
    
    // 2. Generate ephemeral keypair
    uint8_t eph_priv[32], eph_pub[32];
    curve25519_keygen(eph_pub, eph_priv);
    
    // 3. Send CHALLENGE
    RCF_Packet_Header challenge;
    memcpy(challenge.magic, "RCF", 4);
    challenge.version = RCF_PROTOCOL_VERSION;
    challenge.command = RCF_CMD_CHALLENGE;
    challenge.payload_len = 32 + 16; // eph_pub + nonce
    challenge.marker = RCF_MARKER_PUBLIC;
    challenge.session_id = 0;
    
    uint8_t nonce[16];
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)nonce);
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(nonce + 4));
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(nonce + 8));
    HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)(nonce + 12));
    
    usb_send((uint8_t*)&challenge, sizeof(challenge));
    usb_send(eph_pub, 32);
    usb_send(nonce, 16);
    
    // 4. Receive RESPONSE
    uint8_t host_eph_pub[32];
    uint8_t host_response[64];
    usb_receive(host_eph_pub, 32);
    usb_receive(host_response, 64);
    
    // 5. ECDH key exchange
    uint8_t shared_secret[32];
    curve25519_scalar_mult(shared_secret, eph_priv, host_eph_pub);
    
    // 6. Derive session keys
    hkdf_sha256(shared_secret, 32, (uint8_t*)"RCF-v1-Session", 14, 
                session.enc_key, 32, session.mac_key, 32);
    
    // 7. Verify host authentication (if required by license tier)
    if (license_check_feature(RCF_FEATURE_ENTERPRISE)) {
        // Host must present certificate
        // ...
    }
    
    session_active = true;
    session.session_id = generate_session_id();
    session.established_time = get_secure_time();
    
    // 8. Send SESSION confirmation
    RCF_Packet_Header session_confirm;
    // ... encrypted with session key ...
    
    // [RCF-END]
    return true;
}

void protocol_process_command(RCF_Packet_Header* header, uint8_t* payload) {
    // [RCF-START][M-PROTOCOL-DISPATCH]
    switch (header->command) {
        case RCF_CMD_EXECUTE_ACODE:
            if (header->marker == RCF_MARKER_RESTRICTED) {
                rcf_vm_execute("AuroraRemoteModule", payload, header->payload_len);
            }
            break;
        // ... (other commands) ...
    }
    // [RCF-END]
}

bool protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker) {
    // [RCF-START][M-DATA-DISPATCH]
    if (!session_active) return false;
    
    // Check license for marker permission
    if (marker == RCF_MARKER_RESTRICTED && !license_check_feature(RCF_FEATURE_ENTERPRISE)) {
        return false;
    }
    
    // Encrypt based on marker
    uint8_t encrypted[256];
    uint16_t encrypted_len;
    
    switch (marker) {
        case RCF_MARKER_PUBLIC:
            // No encryption, just framing
            memcpy(encrypted, data, len);
            encrypted_len = len;
            break;
        case RCF_MARKER_PROTECTED:
        case RCF_MARKER_RESTRICTED:
            // AES-256-GCM with session key
            aes256_gcm_encrypt(session.enc_key, data, len, encrypted, &encrypted_len);
            break;
    }
    
    RCF_Packet_Header header;
    memcpy(header.magic, "RCF", 4);
    header.version = RCF_PROTOCOL_VERSION;
    header.command = RCF_CMD_DATA;
    header.payload_len = encrypted_len;
    header.marker = marker;
    header.session_id = session.session_id;
    
    // Compute GCM tag for header
    gcm_compute_tag(session.mac_key, (uint8_t*)&header, sizeof(header) - 16, header.auth_tag);
    
    usb_send((uint8_t*)&header, sizeof(header));
    usb_send(encrypted, encrypted_len);
    // [RCF-END]
    
    return true;
}