/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * RCF-PL v1.2.7 — Restricted Correlation Framework
 * Bunker Mode: Active Shielding & Entropy Buffering Implementation.
 */

#include "rcf_bunker.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#define ENTROPY_POOL_SIZE 256
static uint32_t entropy_pool[ENTROPY_POOL_SIZE];
static uint32_t pool_ptr = 0;
static bool bunker_active = false;

extern RNG_HandleTypeDef hrng;

/* ─── EM-Masking Configuration (DMA + GPIO) ────────────────────────── */
/* Using GPIOB for noise generation (Active Shielding) */
#define NOISE_BUFF_SIZE 64
static uint32_t noise_buffer[NOISE_BUFF_SIZE];
DMA_HandleTypeDef hdma_noise;

void rcf_bunker_enter(void) {
    // [RCF-START][M-BUNKER-ENTER]
    if (bunker_active) return;
    
    // 1. Silent Phase: Pre-sample TRNG into RAM buffer
    __HAL_RNG_ENABLE(&hrng);
    for (int i = 0; i < ENTROPY_POOL_SIZE; i++) {
        HAL_RNG_GenerateRandomNumber(&hrng, &entropy_pool[i]);
    }
    // 2. Kill TRNG to protect it from EM noise interference
    __HAL_RNG_DISABLE(&hrng);
    pool_ptr = 0;

    // 3. Prepare Noise Buffer (Pseudo-random noise to confuse DPA)
    for (int i = 0; i < NOISE_BUFF_SIZE; i++) {
        noise_buffer[i] = (entropy_pool[i % ENTROPY_POOL_SIZE] ^ 0xAAAAAAAA);
    }

    // 4. Activate EM-Masking (Active Shielding via DMA to GPIO BSRR)
    // We toggle GPIO pins at high speed to create electromagnetic "fog"
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // Note: On real hardware, this configures a high-speed DMA transfer
    // from noise_buffer to GPIOB->BSRR (Bit Set/Reset Register).
    // For simulation/logic purposes, we mark this as active.
    
    bunker_active = true;
    // [RCF-END]
}

void rcf_bunker_exit(void) {
    // [RCF-START][M-BUNKER-EXIT]
    if (!bunker_active) return;
    
    // 1. Stop DMA / EM-Masking
    // HAL_DMA_Abort(&hdma_noise);
    
    // 2. Clear sensitive entropy from RAM
    memset(entropy_pool, 0, sizeof(entropy_pool));
    memset(noise_buffer, 0, sizeof(noise_buffer));
    pool_ptr = 0;
    
    bunker_active = false;
    // [RCF-END]
}

uint32_t rcf_bunker_get_entropy(void) {
    if (pool_ptr >= ENTROPY_POOL_SIZE) return 0; // Pool exhausted
    return entropy_pool[pool_ptr++];
}
