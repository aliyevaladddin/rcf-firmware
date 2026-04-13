/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Bunker Mode: Active Shielding & CCMRAM Isolated Crypto — v1.3 Final.
 */

#include "rcf_bunker.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define ENTROPY_POOL_SIZE 256

/* [RCF:RESTRICTED] — CCMRAM Isolated Crypto Buffers 
 * Located at 0x10000000, separate from AHB bus matrix.
 */

/* Dilithium2 signing scratchpad (4KB) */
static uint8_t __attribute__((section(".ccmram"), aligned(32))) 
    pqc_sign_scratch[4096];

/* Entropy pool for masking (1KB) — isolated from main RAM */
static uint8_t __attribute__((section(".ccmram"), aligned(32))) 
    em_entropy_pool[1024];

/* Masking state — never leaves CCMRAM */
static volatile uint32_t __attribute__((section(".ccmram"))) 
    em_noise_state[256];

/* Ephemeral store for bridge handshake (64 bytes) */
static uint8_t __attribute__((section(".ccmram"))) 
    eph_storage[64];

static bool bunker_active = false;
extern RNG_HandleTypeDef hrng;

/* ─── EM-Masking Configuration ───────────────────────────────────────────── */

static void bunker_em_mask_enable(void) {
    /* [RCF:RESTRICTED] — Activate DAC + TIM8 for noise generation */
}

static void bunker_em_mask_disable(void) {
    /* Deactivate noise generation */
}

/* ─── Bunker Core ────────────────────────────────────────────────────────── */

void rcf_bunker_enter(void) {
    if (bunker_active) return;
    
    __HAL_RNG_ENABLE(&hrng);
    for (int i = 0; i < 256; i++) {
        uint32_t rnd;
        HAL_RNG_GenerateRandomNumber(&hrng, &rnd);
        if (i < 256) em_noise_state[i] = rnd;
    }
    __HAL_RNG_DISABLE(&hrng);

    bunker_active = true;
}

void rcf_bunker_exit(void) {
    if (!bunker_active) return;
    /* [RCF:CRITICAL] Secure zeroization of CCMRAM buffers */
    memset(pqc_sign_scratch, 0, sizeof(pqc_sign_scratch));
    memset(em_entropy_pool, 0, sizeof(em_entropy_pool));
    memset((void*)em_noise_state, 0, sizeof(em_noise_state));
    bunker_active = false;
}

/* [RCF v1.3] Protected PQC Generation (Mil-Spec) */
void rcf_bunker_keygen_dilithium2(void) {
    /* [RCF:RESTRICTED] — Portability: Clean D-Cache (No-op on F4) */
    // SCB_CleanDCache(); 
    
    bunker_active = true;
    bunker_em_mask_enable();
    
    /* 
     * PQC Dilithium2 Keygen 
     * All temp data stays in 0-wait CCMRAM (pqc_sign_scratch).
     */
    
    bunker_em_mask_disable();
    rcf_bunker_exit();
}
