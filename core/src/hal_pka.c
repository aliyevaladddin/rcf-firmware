/* 
 * [RCF:PROTECTED]
 * hal_pka.c — Sentinel PKA Peripheral Driver & Hardware Mock (RC2-LS)
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "hal_pka.h"
#include <string.h>
#include <stdio.h>

/* Internal PKA Storage Mock (Registers) */
static uint8_t pka_regs_storage[0x100]; 

/* Helper to access registers as uint32_t pointers */
#define PKA_REG(offset) (*(volatile uint32_t*)(pka_regs_storage + (offset)))

void hal_pka_init(void) {
    memset(pka_regs_storage, 0, sizeof(pka_regs_storage));
}

/**
 * @brief HARDWARE MOCK LOGIC
 * Intercepts START bit and simulates ASIC processing.
 */
static void _hal_pka_emulate_hardware(void) {
    // [PKA] Transition to BUSY
    PKA_REG(PKA_REG_STAT) |= PKA_STAT_BUSY;
    PKA_REG(PKA_REG_STAT) &= ~PKA_STAT_DONE;
    
    // Simulate slight "hardware" delay
    uint32_t cmd = PKA_REG(PKA_REG_CMD);
    printf("[PKA] HW Accelerator: Operation %lu started...\n", (unsigned long)cmd);

    // Read pointers from simulated registers
    uint8_t* sig = (uint8_t*)(uintptr_t)PKA_REG(PKA_REG_DATA_IN0);
    
    // Pattern verification (Mock Logic)
    if (sig && sig[0] == 0xDE && sig[1] == 0xAD) {
        printf("[PKA] Result: SIGNATURE VALID (Dilithium2)\n");
        PKA_REG(PKA_REG_RESULT) = 0; // Success
    } else {
        printf("[PKA] Result: SIGNATURE INVALID\n");
        PKA_REG(PKA_REG_RESULT) = 1; // Failure
    }

    // [PKA] Complete operation
    PKA_REG(PKA_REG_STAT) &= ~PKA_STAT_BUSY;
    PKA_REG(PKA_REG_STAT) |= PKA_STAT_DONE;
    
    // Clear START bit (hardware acknowledgment)
    PKA_REG(PKA_REG_CTRL) &= ~PKA_CTRL_START;
}

/* --- Aladdin's Atomic Spec Implementation --- */

void hal_pka_load_data(uint32_t reg_offset, const uint8_t* data, uint32_t len) {
    // For pointers, we store the address. For LEN, we store the value.
    if (reg_offset == PKA_REG_LEN_MSG) {
        PKA_REG(reg_offset) = len;
    } else {
        PKA_REG(reg_offset) = (uint32_t)(uintptr_t)data;
    }
}

void hal_pka_start(void) {
    PKA_REG(PKA_REG_CTRL) |= PKA_CTRL_START;
    PKA_REG(PKA_REG_CMD) = PKA_CMD_DILITHIUM2_VERIFY;
    
    // Hook for CI Simulation
    _hal_pka_emulate_hardware();
}

bool hal_pka_is_ready(void) {
    return (PKA_REG(PKA_REG_STAT) & PKA_STAT_DONE) != 0;
}

void hal_pka_read_result(uint32_t* out_result) {
    if (out_result) {
        *out_result = PKA_REG(PKA_REG_RESULT);
    }
}

/* Legacy wrapper (can be used but rcf_pqc.c will use atomic ones) */
int hal_pka_verify_dilithium2(const uint8_t* sig, const uint8_t* msg, uint32_t len, const uint8_t* pk) {
    hal_pka_load_data(PKA_REG_DATA_IN0, sig, 0);
    hal_pka_load_data(PKA_REG_DATA_IN1, msg, 0);
    hal_pka_load_data(PKA_REG_DATA_IN2, pk, 0);
    hal_pka_load_data(PKA_REG_LEN_MSG, NULL, len);
    
    hal_pka_start();
    
    while (!hal_pka_is_ready());
    
    uint32_t result;
    hal_pka_read_result(&result);
    return (result == 0) ? 0 : -1;
}
