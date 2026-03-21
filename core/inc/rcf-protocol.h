// [RCF:PROTECTED] — RCF Protocol header
#ifndef RCF_PROTOCOL_H
#define RCF_PROTOCOL_H

#include "rcf_config.h"
#include <stdbool.h>
#include <stdint.h>

// Protocol commands
#define RCF_CMD_HELLO           0x01
#define RCF_CMD_CHALLENGE       0x02
#define RCF_CMD_RESPONSE        0x03
#define RCF_CMD_SESSION         0x04
#define RCF_CMD_DATA            0x05
#define RCF_CMD_AUDIT           0x06
#define RCF_CMD_PILL_OFF        0xFF

// Markers for data classification
#define RCF_MARKER_PUBLIC       0x01
#define RCF_MARKER_PROTECTED    0x02
#define RCF_MARKER_RESTRICTED   0x03

typedef struct {
    uint8_t  magic[4];           // "RCF\0"
    uint16_t version;
    uint8_t  command;
    uint16_t payload_len;
    uint8_t  marker;             // PUBLIC/PROTECTED/RESTRICTED
    uint16_t session_id;
    uint8_t  reserved[2];
    uint8_t  auth_tag[16];       // GCM tag
} RCF_Packet_Header;

// Lifecycle
bool protocol_init(void);
void protocol_process(void);

// Session management
bool protocol_establish_session(void);
bool protocol_close_session(void);
bool protocol_is_session_active(void);

// Data transmission
bool protocol_send_data(const uint8_t* data, uint16_t len, uint8_t marker);
bool protocol_receive_data(uint8_t* out_buffer, uint16_t max_len, uint16_t* out_received);

#endif // RCF_PROTOCOL_H