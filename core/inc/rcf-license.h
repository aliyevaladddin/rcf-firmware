// [RCF:RESTRICTED] — License validation header
#ifndef RCF_LICENSE_H
#define RCF_LICENSE_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t  code_fingerprint[32];   // SHA-256(.text || .rodata)
    uint64_t issue_timestamp;
    uint64_t expiration_timestamp;
    uint8_t  feature_flags;
    uint8_t  device_binding[12];     // STM32 UID
    uint8_t  audit_anchor[32];         // SHA-256 последнего аудита
    uint8_t  reserved[11];
} RCF_License_Payload;

typedef struct {
    uint8_t  version;
    uint8_t  payload[128];             // Encrypted RCF_License_Payload
    uint8_t  hmac[32];                 // HMAC-SHA256(device_secret, payload)
    uint8_t  signature[64];            // Ed25519(RCF_Authority, payload || hmac)
} RCF_License_Block;

// Validation
bool license_init(void);
bool license_validate(void);
bool license_is_valid(void);

// Feature checking
bool license_check_feature(uint8_t feature_flag);
uint8_t license_get_features(void);

// Code integrity
bool license_verify_code_fingerprint(void);

// Renewal
bool license_update(const RCF_License_Block* new_license);

#endif // RCF_LICENSE_H