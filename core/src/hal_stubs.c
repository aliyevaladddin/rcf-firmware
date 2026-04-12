/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 */

#include "stm32f4xx_hal.h"
#include "rcf_led.h"
#include <stdio.h>
#include <unistd.h>  /* For usleep in CI mode */
#include <string.h>

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
    /* Verify entropy (simple check for constants or fault conditions) */
    if (trng_test[0] == trng_test[1] || trng_test[0] == 0 || trng_test[0] == 0xFFFFFFFF) {
        // [RCF:CRITICAL] TRNG Fault detected
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

/* [RCF:CRITICAL] GCC Stack protection failure handler */
void __stack_chk_fail(void) {
    extern void trigger_pill_off(uint8_t reason);
    trigger_pill_off(0x12); // PILL_OFF_TAMPER_CODE
}

void HAL_Init(void) {
    // Stub
}

void SystemClock_Config(void) {
    // Stub
}

uint32_t HAL_GetTick(void) {
    static uint32_t tick = 0;
    return tick++;
}

void HAL_Delay(uint32_t Delay) {
#ifdef RCF_VM_CI_MODE
    usleep(Delay * 1000); // 1ms = 1000us
#endif
}

void HAL_RNG_Init(RNG_HandleTypeDef* phrng) {
    phrng->Instance = RNG;
}

void HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* phrng, uint32_t* random32) {
    static uint32_t seed = 0x12345678;
    seed = seed * 1103515245 + 12345;
    *random32 = seed;
}

void __HAL_RNG_ENABLE(RNG_HandleTypeDef* phrng) { (void)phrng; }
void __HAL_RNG_DISABLE(RNG_HandleTypeDef* phrng) { (void)phrng; }

void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg) { (void)hiwdg; }

/* RTC Stubs */
void HAL_RTC_GetTime(RTC_HandleTypeDef* phrtc, RTC_TimeTypeDef* sTime, uint32_t Format) {
    (void)phrtc; (void)Format;
    sTime->Hours = 12; sTime->Minutes = 0; sTime->Seconds = 0;
}

void HAL_RTC_GetDate(RTC_HandleTypeDef* phrtc, RTC_DateTypeDef* sDate, uint32_t Format) {
    (void)phrtc; (void)Format;
    sDate->Year = 26; sDate->Month = 4; sDate->Date = 12;
}

/* Timechain Helpers */
uint64_t rtc_to_unix(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate) {
    (void)sTime; (void)sDate;
    return 1712923200; // Fixed mock timestamp: 2026-04-12 12:00:00
}

int16_t get_internal_temperature(void) { return 25; }
uint16_t get_vbat_voltage(void) { return 3300; }

/* Mocking missing module functions to satisfy linker */
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

/* Mocking printf redirection for QEMU serial */
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        // UART stub
    }
    return len;
}
