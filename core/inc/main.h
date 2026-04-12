/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * NOTICE: This file is protected under RCF-PL v1.3
 * Main entry point prototypes and configuration.
 */

#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* [RCF v1.3] Hardware Handles (defined in hal_stubs.c) */
extern RNG_HandleTypeDef hrng;
extern IWDG_HandleTypeDef hiwdg;

/* [RCF v1.3] CI/QEMU Helpers */
#ifdef RCF_VM_CI_MODE
    #define RCF_CI_LOG(msg) printf("[RCF-CI] %s\n", msg)
#else
    #define RCF_CI_LOG(msg) ((void)0)
#endif

/* System Lifecycle */
void SystemClock_Config(void);
void Error_Handler(void);

/* Hardware Integrity Stubs */
void trng_health_check(void);
void stack_canary_init(void);
void tamper_init(void);
void usb_init(void);

#endif /* MAIN_H */
