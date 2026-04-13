/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Dual-UART Hardware Abstraction Layer (Mil-Spec RC2-LS)
 * USART1: Bridge Data (0x40011000)
 * USART2: Debug Console (0x40004400)
 * Includes components: RNG, RTC, IWDG, Sensors.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Hardware Base Addresses (STM32F407) */
#define USART1_BASE      0x40011000U
#define USART2_BASE      0x40004400U
#define USART_DR_OFFSET  0x04U
#define USART_SR_OFFSET  0x00U

/* Register Access Macros */
#define USART1_DR  (*(volatile uint32_t*)(USART1_BASE + USART_DR_OFFSET))
#define USART1_SR  (*(volatile uint32_t*)(USART1_BASE + USART_SR_OFFSET))
#define USART2_DR  (*(volatile uint32_t*)(USART2_BASE + USART_DR_OFFSET))
#define USART2_SR  (*(volatile uint32_t*)(USART2_BASE + USART_SR_OFFSET))

/* Status Flags */
#define USART_SR_TXE     (1U << 7)  /* Transmit Data Register Empty */
#define USART_SR_RXNE    (1U << 5)  /* Read Data Register Not Empty */

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
    (void)ms; 
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

/* Sensors */
int16_t get_internal_temperature(void) { return 250; }
uint32_t get_vbat_voltage(void) { return 3300; }

/* ─── USART1: Bridge Data Interface (Binary) ─────────────────────────────── */

void hal_uart_send(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        while (!(USART1_SR & USART_SR_TXE));
        USART1_DR = data[i];
    }
}

bool hal_uart_receive(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        while (!(USART1_SR & USART_SR_RXNE));
        data[i] = (uint8_t)USART1_DR;
    }
    return true;
}

/* ─── USART2: Debug Console Interface (Text) ─────────────────────────────── */

/**
 * @brief Newlib _write redirect for printf/stdout mapping to USART2.
 * High priority implementation for Debugging via Hardware USART.
 */
int _write(int file, char* ptr, int len) {
    (void)file;
    for (int i = 0; i < len; i++) {
        while (!(USART2_SR & USART_SR_TXE));
        USART2_DR = (uint8_t)ptr[i];
    }
    return len;
}

/* Memory management symbols for syscalls.c / malloc */
char end_val;
char *end = &end_val;
