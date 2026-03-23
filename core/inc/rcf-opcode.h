/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * RCF-PL v1.2.7 — Restricted Correlation Framework
 * A-Code (Aurora Native Bytecode) OpCode Definitions.
 */

#ifndef RCF_OPCODE_H
#define RCF_OPCODE_H

#include <stdint.h>

/* ───── Core Instructions ───── */
#define OP_INIT_MOD          0x01
#define OP_HALT              0x99

/* ───── Identity & Security ───── */
#define OP_IDENTITY_GEN      0x05
#define OP_KEM_EXCHANGE      0x06
#define OP_PQC_SET_REGIME    0x07

/* ───── Virtual File System (VFS) ───── */
#define OP_VFS_STORE         0x10
#define OP_VFS_FETCH         0x11

/* ───── System Bus / Peripheral ───── */
#define OP_BUS_PUB           0x20
#define OP_BUS_SUB           0x21

/* ───── Biometrics & Sensors ───── */
#define OP_PULSE_EMIT        0x40
#define OP_SYS_BIOMETRICS    0x45
#define OP_FEEL_STATE        0x50

/* ───── Reflexes & Instincts ───── */
#define OP_INSTINCT_TRIGGER  0x60
#define OP_REFLEX_ACTION     0x65

/* ───── High-Level Intents ───── */
#define OP_INTUITION_PREDICT 0x70
#define OP_LUME_VOICE        0xA0
#define OP_LUME_SUGGEST      0xA5

/* ───── Verification ───── */
#define OP_PURITY_VERIFY     0xFF

#endif /* RCF_OPCODE_H */
