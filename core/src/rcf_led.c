/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * LED Control — CI/QEMU Stub.
 */

#include "rcf_led.h"
#include <stdio.h>

void led_init(void) {
    /* STM32 GPIO Init for LEDs */
}

void led_set_pattern(LED_Pattern pattern) {
    /* Update PWM/GPIO according to pattern */
#ifdef RCF_VM_CI_MODE
    printf("[RCF-CI] LED Pattern set to: %d\n", pattern);
#endif
}

void led_set_void_black(void) {
    /* Immediately zero all GPIO pins for LEDs */
#ifdef RCF_VM_CI_MODE
    printf("[RCF-CI] LED VOID BLACK (Blackout activated)\n");
#endif
}
