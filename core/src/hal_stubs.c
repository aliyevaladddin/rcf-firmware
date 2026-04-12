/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Implementation Stubs for CI/QEMU Verification.
 */

#include "stm32f4xx_hal.h"
#include <stdio.h>

/* Global instances used in main.c */
RNG_HandleTypeDef hrng;
IWDG_HandleTypeDef hiwdg;

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

void HAL_RNG_Init(RNG_HandleTypeDef* hrng) {
    // Stub
}

void HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* hrng, uint32_t* random32) {
    *random32 = 0x12345678; // Mock random
}

void __HAL_RNG_ENABLE(RNG_HandleTypeDef* hrng) { (void)hrng; }
void __HAL_RNG_DISABLE(RNG_HandleTypeDef* hrng) { (void)hrng; }

void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg) { (void)hiwdg; }

/* Mocking missing module functions to satisfy linker */
void led_init(void) {}
void led_set_pattern(int p) { (void)p; }
void tamper_init(void) {}
void usb_init(void) {}
void pulse_init(void) {}
bool pulse_check_activation(void) { return false; }
bool pulse_validate_liveness(void) { return false; }
int  get_failed_attempts(void) { return 0; }
void enter_provisioning_mode(void) {}
void process_dos_request(uint8_t* r, uint16_t l) { (void)r; (void)l; }
void protocol_send_dos_bootstrap(void) {}

/* Mocking printf redirection for QEMU serial */
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        // In QEMU netduinoplus2, UART output can be caught this way
        // or via semihosting. For -nographic, it often maps to stdout.
    }
    return len;
}
