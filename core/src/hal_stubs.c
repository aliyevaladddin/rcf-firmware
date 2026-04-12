/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 */

#include "stm32f4xx_hal.h"
#include "rcf_led.h"
#include <stdio.h>
#include <unistd.h>
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

void HAL_Delay(uint32_t Delay) {
#ifdef RCF_VM_CI_MODE
    usleep(Delay * 1000);
#endif
}

void HAL_RNG_Init(RNG_HandleTypeDef* phrng) {
    phrng->Instance = RNG;
}

/* [FIX] Return HAL_StatusTypeDef to satisfy timechain.c */
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

/* ───────────────────────────────────────────────────────────────────────── */

void led_init(void) {}
void led_set_pattern(LED_Pattern p) { (void)p; }
void led_set_void_black(void) {}
void tamper_init(void) {}
void usb_init(void) {}
void pulse_init(void) {}
bool pulse_check_activation(void) { return false; }
bool pulse_validate_liveness(void) { return false; }
uint8_t get_failed_attempts(void) { return 0; }
void enter_provisioning_mode(void) {}
void process_dos_request(uint8_t* r, uint16_t l) { (void)r; (void)l; }
void protocol_send_dos_bootstrap(void) {}
