/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 */

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

RNG_HandleTypeDef hrng;
IWDG_HandleTypeDef hiwdg;
RTC_HandleTypeDef hrtc;
uint32_t __stack_chk_guard = 0xDEADBEEF;

void HAL_Init(void) {}
void SystemClock_Config(void) {}

void HAL_RNG_Init(RNG_HandleTypeDef* phrng) { (void)phrng; }
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* phrng, uint32_t* random32) {
    (void)phrng;
    *random32 = (uint32_t)rand();
    return HAL_OK;
}

void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg) { (void)hiwdg; }

/* ─── Bridge Helpers ─────────────────────────────────────────────────────── */

int16_t get_internal_temperature(void) {
    return 250; /* 25.0°C */
}

uint32_t get_vbat_voltage(void) {
    return 3300; /* 3.3V */
}

/* ─── UART for Bridge ────────────────────────────────────────────────────── */

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

/* ─── Other Stubs ────────────────────────────────────────────────────────── */

void trigger_pill_off(uint8_t reason) {
    printf("[RCF-PILLOFF] Reason: %02X\n", reason);
#ifdef RCF_VM_CI_MODE
    exit(reason);
#endif
}

void rcf_audit_log(uint8_t event, uint32_t data) {
    printf("[AUDIT] %02X: %08X\n", event, (unsigned int)data);
}
