/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Aurora Virtual Machine (A-VM) Execution Engine.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#ifndef RCF_VM_H
#define RCF_VM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Constants ──────────────────────────────────────────────────────────── */

#define RCF_DILITHIUM2_SIG_SIZE     2420U
#define RCF_DILITHIUM2_PK_SIZE      1312U
#define RCF_VM_MAX_MODULE_SIZE      (64U * 1024U)   /* 64 KB max .acode     */
#define RCF_VM_MAX_MODULE_NAME      64U
#define RCF_VM_MAX_CALL_DEPTH       16U             /* Stack overflow guard  */

/* ─── Execution result ───────────────────────────────────────────────────── */

typedef enum {
    VM_OK                   =  0,
    VM_ERR_NULL_INPUT       = -1,
    VM_ERR_TOO_SMALL        = -2,   /* Module smaller than signature         */
    VM_ERR_TOO_LARGE        = -3,   /* Module exceeds RCF_VM_MAX_MODULE_SIZE */
    VM_ERR_SIG_INVALID      = -4,   /* Dilithium2 verification failed        */
    VM_ERR_UNKNOWN_OPCODE   = -5,   /* Unrecognised opcode encountered       */
    VM_ERR_STACK_OVERFLOW   = -6,   /* Call depth exceeded                   */
    VM_ERR_BUNKER_FAIL      = -7,   /* Bunker enter/exit mismatch            */
    VM_ERR_PILL_OFF         = -8,   /* Execution terminated by REFLEX_ACTION */
} RCF_VM_Result;

/* ─── Module descriptor ──────────────────────────────────────────────────── */

typedef struct {
    const char*    name;            /* Human-readable module name            */
    const uint8_t* envelope;        /* [Sig (2420B)] + [A-Code bytecode]     */
    uint32_t       envelope_size;   /* Total envelope length                 */
} RCF_Module;

/* ─── Public API ─────────────────────────────────────────────────────────── */

/**
 * Execute a signed A-Code module.
 *
 * Verifies the Dilithium2 signature against the Sentinel MPK before
 * executing a single opcode. On signature failure, triggers pill-off.
 *
 * @param name     Module name for audit log (max RCF_VM_MAX_MODULE_NAME).
 * @param bytecode RCF envelope: [Dilithium2 sig][A-Code opcodes].
 * @param size     Total envelope size in bytes.
 * @return         VM_OK on clean HALT, negative error code otherwise.
 */
RCF_VM_Result rcf_vm_execute(const char*    name,
                              const uint8_t* bytecode,
                              uint32_t       size);

/**
 * Verify module signature only (no execution).
 * Useful for pre-flight checks before loading into A-VM.
 */
RCF_VM_Result rcf_vm_verify(const uint8_t* bytecode, uint32_t size);

/**
 * Returns true if the VM is currently inside a Bunker context.
 * Used by external watchdog to detect stuck Bunker state.
 */
bool rcf_vm_in_bunker(void);

/**
 * Force-exit Bunker mode and reset VM state.
 * Called by watchdog or emergency handler.
 */
void rcf_vm_emergency_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* RCF_VM_H */
