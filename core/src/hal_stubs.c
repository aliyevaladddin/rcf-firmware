/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 * Sync: rcf-firmware RC2 Hardened.
 */

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global instances used in main.c and modules */
RNG_HandleTypeDef hrng;
IWDG_HandleTypeDef hiwdg;
RTC_HandleTypeDef hrtc;
uint32_t __stack_chk_guard = 0xDEADBEEF;

/* Missing IRQ Handlers for startup code */
void WWDG_IRQHandler(void) {}
void PVD_IRQHandler(void) {}
void TAMP_STAMP_IRQHandler(void) {}
void RTC_WKUP_IRQHandler(void) {}

/* Hardware Inits (Stubs) */
void HAL_Init(void) {}
void SystemClock_Config(void) {}
void tamper_init(void) {}
void usb_init(void) {}

/* Stack & TRNG security (Stubs) */
void trng_health_check(void) {}
void stack_canary_init(void) {}
void __stack_chk_fail(void) {
    extern void trigger_pill_off(uint8_t reason);
    trigger_pill_off(0x12);
}

/* Time & Delay */
uint32_t HAL_GetTick(void) {
    static uint32_t tick = 0;
    return tick++;
}

void HAL_Delay(uint32_t ms) {
    (void)ms; /* In CI, we don't really want to wait */
}

/* RTC & Timechain */
void HAL_RTC_GetTime(RTC_HandleTypeDef* phrtc, RTC_TimeTypeDef* sTime, uint32_t Format) {
    (void)phrtc; (void)Format;
    sTime->Hours = 12; sTime->Minutes = 0; sTime->Seconds = 0;
}

void HAL_RTC_GetDate(RTC_HandleTypeDef* phrtc, RTC_DateTypeDef* sDate, uint32_t Format) {
    (void)phrtc; (void)Format;
    sDate->Year = 26; sDate->Month = 4; sDate->Date = 13;
}

uint64_t rtc_to_unix(RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate) {
    (void)sTime; (void)sDate;
    return 1713000000ULL;
}

/* RNG */
void HAL_RNG_Init(RNG_HandleTypeDef* phrng) { phrng->Instance = (void*)1; }
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* phrng, uint32_t* random32) {
    (void)phrng;
    *random32 = (uint32_t)rand();
    return HAL_OK;
}

void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg) { (void)hiwdg; }

/* ─── Bridge Helpers ─────────────────────────────────────────────────────── */

int16_t get_internal_temperature(void) { return 250; }
uint32_t get_vbat_voltage(void) { return 3300; }

void hal_uart_send(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) putchar(data[i]);
    fflush(stdout);
}

bool hal_uart_receive(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        int c = getchar();
        if (c == EOF) return false;
        data[i] = (uint8_t)c;
    }
    return true;
}

/* Memory management symbol for syscalls.c */
char end_val;
char *end = &end_val;
