/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Bunker Mode: Active Shielding & Entropy Buffering Definitions.
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

/**
 * @brief Generates a Dilithium2 keypair inside active Bunker mode.
 */
void rcf_bunker_keygen_dilithium2(void);

/**
 * @brief Securely stores ephemeral key material during bridge handshake.
 */
void rcf_bunker_store_ephemeral(const uint8_t* key, uint32_t len);

/**
 * @brief Retrieves stored ephemeral key material and clears the buffer.
 */
void rcf_bunker_retrieve_ephemeral(uint8_t* out_key, uint32_t len);


#endif /* RCF_BUNKER_H */
