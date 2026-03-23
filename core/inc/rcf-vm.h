/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * RCF-PL v1.2.7 — Restricted Correlation Framework
 * Aurora Virtual Machine (A-VM) Execution Engine.
 */

#ifndef RCF_VM_H
#define RCF_VM_H

#include <stdint.h>

/**
 * @brief Executes a signed A-Code module.
 * 
 * @param name Module name for logging/tracing.
 * @param bytecode Pointer to the RCF envelope [Signature + OpCodes].
 * @param size Total size of the envelope.
 */
void rcf_vm_execute(const char* name, const uint8_t* bytecode, uint32_t size);

#endif /* RCF_VM_H */
