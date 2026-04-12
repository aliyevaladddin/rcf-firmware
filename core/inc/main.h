/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * CI/QEMU conditional compilation & Boot Sequence — v1.3 RC2.
 */

#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"
#include "rcf_config.h"

/* ─── CI Mode detection ──────────────────────────────────────────────────── */

#ifdef RCF_VM_CI_MODE
  #include <stdio.h>  /* printf available in QEMU */
  #define RCF_CI_LOG(msg) printf("[RCF-CI] %s\n", msg)
#else
  #define RCF_CI_LOG(msg) ((void)0)
#endif

/* ─── Security Prototypes ────────────────────────────────────────────────── */

/* TRNG entropy validation */
void trng_health_check(void);

/* Dynamic stack protection randomization */
void stack_canary_init(void);

/* Vault initialization with fault-injection resistance */
bool vault_init_with_retry(uint8_t max_retries);

/* Boot integrity fallback */
void Error_Handler(void);

/* System configuration */
void SystemClock_Config(void);

#endif /* MAIN_H */
