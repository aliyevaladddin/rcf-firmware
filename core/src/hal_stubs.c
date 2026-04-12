/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 */

#include "stm32f4xx_hal.h"
#include "rcf_led.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Global instances used in main.c and modules */
RNG_HandleTypeDef hrng;
IWDG_HandleTypeDef hiwdg;
RTC_HandleTypeDef hrtc;
uint32_t __stack_chk_guard = 0xDEADBEEF;

/* [RCF v1.3] TRNG Health Check — Protect against hardware fault attacks */
void trng_health_check(void) {
    uint32_t trng_test[8];
    for (int i = 0; i < 8; i++) {
        HAL_RNG_GenerateRandomNumber(&hrng, &trng_test[i]);
    }
    if (trng_test[0] == trng_test[1] || trng_test[0] == 0 || trng_test[0] == 0xFFFFFFFF) {
        extern void trigger_pill_off(uint8_t reason);
        trigger_pill_off(0x14); // PILL_OFF_TRNG_FAULT
    }
}

/* [RCF v1.3] Stack Canary Randomization */
void stack_canary_init(void) {
    uint32_t random_val;
    HAL_RNG_GenerateRandomNumber(&hrng, &random_val);
    if (random_val != 0 && random_val != 0xFFFFFFFF) {
        __stack_chk_guard = random_val;
    }
}

void __stack_chk_fail(void) {
    extern void trigger_pill_off(uint8_t reason);
    trigger_pill_off(0x12); // PILL_OFF_TAMPER_CODE
}

void HAL_Init(void) {}
void SystemClock_Config(void) {}

uint32_t HAL_GetTick(void) {
    static uint32_t tick = 0;
    return tick++;
}

/* [FIX] Busy-wait implementation for bare-metal CI */
void HAL_Delay(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 8000U; i++) {
        __asm__ volatile ("nop");
    }
}

void HAL_RNG_Init(RNG_HandleTypeDef* phrng) {
    phrng->Instance = (void*)RNG;
}

HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* phrng, uint32_t* random32) {
    (void)phrng;
    *random32 = (uint32_t)rand();
    return HAL_OK;
}

void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg) { (void)hiwdg; }

void HAL_RTC_GetTime(RTC_HandleTypeDef* phrtc, RTC_TimeTypeDef* sTime, uint32_t Format) {
    (void)phrtc; (void)Format;
    sTime->Hours = 12; sTime->Minutes = 0; sTime->Seconds = 0;
}

void HAL_RTC_GetDate(RTC_HandleTypeDef* phrtc, RTC_DateTypeDef* sDate, uint32_t Format) {
    (void)phrtc; (void)Format;
    sDate->Year = 26; sDate->Month = 4; sDate->Date = 12;
}

/* ─── Emergency Patch: TimeChain Helpers for CI ─────────────────────────── */

uint64_t rtc_to_unix(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate) {
    (void)sTime; (void)sDate;
    return 1712947200ULL;
}

int16_t get_internal_temperature(void) {
    return 250; /* 25.0°C */
}

uint32_t get_vbat_voltage(void) {
    return 3300; /* 3.3V */
}

/* ─── IRQ Handlers ──────────────────────────────────────────────────────── */

void WWDG_IRQHandler(void) {}
void PVD_IRQHandler(void) {}
void TAMP_STAMP_IRQHandler(void) {}
void RTC_WKUP_IRQHandler(void) {}
