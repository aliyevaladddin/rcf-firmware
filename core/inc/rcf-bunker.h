/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * Bunker Mode: Active Shielding & Entropy Buffering.
 */

#ifndef RCF_BUNKER_H
#define RCF_BUNKER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Enters Bunker Mode: Pre-samples TRNG and activates EM-Shielding.
 * This should be called before sensitive PQC operations.
 */
void rcf_bunker_enter(void);

/**
 * @brief Exits Bunker Mode: Disables EM-Shielding and clears entropy buffers.
 */
void rcf_bunker_exit(void);

/**
 * @brief Retrieves a word of "pure" buffered entropy.
 * @return 32-bit random word.
 */
uint32_t rcf_bunker_get_entropy(void);

#endif /* RCF_BUNKER_H */
