/*
 * [RCF:PROTECTED]
 * NOTICE: This file is protected under RCF-PL v1.3
 *
 * rcf_bridge_hsm.h — STM32 (HSM) side bridge interface.
 */

#ifndef RCF_BRIDGE_HSM_H
#define RCF_BRIDGE_HSM_H

#include <stdbool.h>

/**
 * Main process loop for the bridge. 
 * Should be called periodically in the firmware main loop.
 */
void rcf_bridge_hsm_process(void);

/**
 * Returns true if a secure bridge session with dOS is active.
 */
bool rcf_bridge_hsm_is_active(void);

#endif /* RCF_BRIDGE_HSM_H */
