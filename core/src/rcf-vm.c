/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Aurora Virtual Machine (A-VM) — STM32F4 Hardened Implementation.
 *
 * Copyright (c) 2026 Aladdin Aliyev. All rights reserved.
 */

#include "rcf_vm.h"
#include "rcf_opcode.h"
#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_bunker.h"
#include "rcf_pilloff.h"
#include "rcf_audit.h"
#include "rcf-config.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* ─── Logging ────────────────────────────────────────────────────────────── */

#ifdef RCF_VM_DEBUG
  #include <stdio.h>
  #define RCF_VM_LOG(fmt, ...)  printf("[A-VM] " fmt "\n", ##__VA_ARGS__)
#else
  #define RCF_VM_LOG(fmt, ...)  ((void)0)
#endif

/* ─── Module state ───────────────────────────────────────────────────────── */

static bool     vm_in_bunker    = false;
static uint32_t vm_call_depth   = 0U;
static uint32_t last_lume_tick  = 0U;

typedef struct {
    uint32_t instruction_count;
    uint32_t stack_depth_max;
    uint8_t  anomaly_score;
} AVM_ReflexMonitor;

/* ─── Internal helpers ───────────────────────────────────────────────────── */

static bool _detect_side_channel_probe(void) {
    /* [RCF:PROTECTED] — Hardware-dependent ADC/Comp check */
    return false; 
}

static void _reflex_trigger(const char* name, uint8_t reason) {
    rcf_audit_log(EVENT_VM_REFLEX, reason);
    RCF_VM_LOG("REFLEX: Critical anomaly in %s. Reason: %d", name, reason);
    trigger_pill_off(PILL_OFF_TAMPER_CODE);
}

static RCF_VM_Result _integrity_verify(const uint8_t* sig,
                                        const uint8_t* code,
                                        uint32_t       code_len)
{
    /* [RCF:RESTRICTED] — Use explicit Public Key from OTP */
    const uint8_t* mpk_pub = rcf_vault_get_mpk_public();
    
    if (!rcf_vault_is_mpk_locked()) {
        rcf_audit_log(EVENT_TAMPER_VAULT, 0);
        return VM_ERR_SIG_INVALID;
    }

    int result = rcf_pqc_verify(sig, code, (int)code_len, mpk_pub);
    return (result == 0) ? VM_OK : VM_ERR_SIG_INVALID;
}

static inline bool _ip_valid(uint32_t ip, uint32_t code_size, uint32_t needed)
{
    return (ip + needed) <= code_size;
}

/* ─── Execution engine ───────────────────────────────────────────────────── */

/* [RCF:RESTRICTED] */
RCF_VM_Result rcf_vm_execute(const char*    name,
                              const uint8_t* bytecode,
                              uint32_t       size)
{
    /* 1. Pre-execution Security Checks */
    if (!bytecode || !name) return VM_ERR_NULL_INPUT;
    
    /* [RCF v1.3] Envelope Alignment Check (Mil-Spec) */
    if (size % 32 != 0) {
        rcf_audit_log(EVENT_VM_MISALIGNED, size);
        return VM_ERR_TOO_SMALL;
    }

    if (size <= RCF_DILITHIUM2_SIG_SIZE) return VM_ERR_TOO_SMALL;
    if (size > RCF_VM_MAX_MODULE_SIZE)   return VM_ERR_TOO_LARGE;

    if (vm_call_depth >= RCF_VM_MAX_CALL_DEPTH) return VM_ERR_STACK_OVERFLOW;
    vm_call_depth++;

    /* 2. Dilithium2 Signature Verification (Constant-time expected) */
    const uint8_t* sig       = bytecode;
    const uint8_t* code      = bytecode + RCF_DILITHIUM2_SIG_SIZE;
    uint32_t       code_size = size     - RCF_DILITHIUM2_SIG_SIZE;

    rcf_audit_log(EVENT_VM_VERIFY_START, code_size);
    if (_integrity_verify(sig, code, code_size) != VM_OK) {
        rcf_audit_log(EVENT_VM_SIG_FAIL, 0);
        trigger_pill_off(PILL_OFF_TAMPER_CODE);
        vm_call_depth--;
        return VM_ERR_SIG_INVALID;
    }
    rcf_audit_log(EVENT_VM_VERIFY_OK, code_size);

    /* 3. A-VM Hardened Environment Setup */
    AVM_ReflexMonitor reflex = {0, RCF_AVM_MAX_STACK_DEPTH, 0};
    volatile uint32_t stack_guard = 0xDEADBEEFU; // Mil-Spec Stack Canary

    uint32_t      ip     = 0U;
    RCF_VM_Result result = VM_OK;

    /* ── Execution loop ────────────────────────────────────────────────── */
    while (ip < code_size) {
        /* A. Security Watchdogs */
        if (stack_guard != 0xDEADBEEFU) {
             _reflex_trigger(name, 0x04); // REASON_STACK_TAMPER
             result = VM_ERR_PILL_OFF; goto vm_exit;
        }

        if (++reflex.instruction_count > RCF_AVM_MAX_INSTRUCTIONS) {
            _reflex_trigger(name, 0x01); // REASON_INFINITE_LOOP
            result = VM_ERR_PILL_OFF; goto vm_exit;
        }

        if (_detect_side_channel_probe()) {
            _reflex_trigger(name, 0x02); // REASON_SIDE_CHANNEL
            result = VM_ERR_PILL_OFF; goto vm_exit;
        }

        if (!_ip_valid(ip, code_size, 1U)) break;
        uint8_t op = code[ip++];

        switch (op) {
            case OP_INIT_MOD:     rcf_audit_log(EVENT_VM_OP_INIT, 0); break;
            
            case OP_IDENTITY_GEN:
                rcf_bunker_enter(); vm_in_bunker = true;
                rcf_bunker_keygen_dilithium2();
                rcf_bunker_exit(); vm_in_bunker = false;
                rcf_audit_log(EVENT_VM_IDENTITY_REGEN, 0);
                break;

            case OP_VFS_STORE:
                if (!_ip_valid(ip, code_size, 2U)) { result = VM_ERR_UNKNOWN_OPCODE; goto vm_exit; }
                {
                    uint16_t obj_id = (uint16_t)(code[ip] | (code[ip+1] << 8)); ip += 2;
                    rcf_vault_vfs_store(obj_id);
                }
                break;

            case OP_VFS_FETCH:
                if (!_ip_valid(ip, code_size, 2U)) { result = VM_ERR_UNKNOWN_OPCODE; goto vm_exit; }
                {
                    uint16_t obj_id = (uint16_t)(code[ip] | (code[ip+1] << 8)); ip += 2;
                    rcf_vault_vfs_fetch(obj_id);
                }
                break;

            case OP_BUS_PUB:      rcf_audit_log(EVENT_VM_BUS_PUB, 0); break;
            case OP_EXT_MOUNT:
                if (!_ip_valid(ip, code_size, 1U)) { result = VM_ERR_UNKNOWN_OPCODE; goto vm_exit; }
                {
                    uint8_t slot = code[ip++]; rcf_audit_log(EVENT_VM_EXT_MOUNT, slot);
                }
                break;

            case OP_REFLEX_ACTION:
                rcf_vault_emergency_wipe(); result = VM_ERR_PILL_OFF; goto vm_exit;

            case OP_LUME_VOICE:
                /* [RCF-MilSpec] — Covert Channel / Timing Anomaly Detection */
                {
                    uint32_t now = HAL_GetTick();
                    if (last_lume_tick != 0 && (now - last_lume_tick < 10000)) {
                        _reflex_trigger(name, 0x05); // REASON_TIMING_ANOMALY
                        result = VM_ERR_PILL_OFF; goto vm_exit;
                    }
                    last_lume_tick = now;
                }
                rcf_audit_log(EVENT_VM_LUME_VOICE, 0);
                break;

            case OP_HALT:         rcf_audit_log(EVENT_VM_HALT, ip); goto vm_exit;

            default:
                rcf_audit_log(EVENT_VM_UNKNOWN_OP, op);
                #ifdef RCF_VM_STRICT
                result = VM_ERR_UNKNOWN_OPCODE; goto vm_exit;
                #endif
                break;
        }
    }

vm_exit:
    if (vm_in_bunker) { rcf_bunker_exit(); vm_in_bunker = false; }
    vm_call_depth--;
    return result;
}

/* ─── Other Public API as before ... ─────────────────────────────────────── */
bool rcf_vm_in_bunker(void) { return vm_in_bunker; }
void rcf_vm_emergency_reset(void) {
    if (vm_in_bunker) { rcf_bunker_exit(); vm_in_bunker = false; }
    vm_call_depth = 0U; rcf_audit_log(EVENT_VM_EMERGENCY_RESET, 0);
}
RCF_VM_Result rcf_vm_verify(const uint8_t* bytecode, uint32_t size) {
    if (!bytecode || size % 32 != 0) return VM_ERR_TOO_SMALL;
    if (size <= RCF_DILITHIUM2_SIG_SIZE) return VM_ERR_TOO_SMALL;
    const uint8_t* sig = bytecode;
    const uint8_t* code = bytecode + RCF_DILITHIUM2_SIG_SIZE;
    return _integrity_verify(sig, code, size - RCF_DILITHIUM2_SIG_SIZE);
}
