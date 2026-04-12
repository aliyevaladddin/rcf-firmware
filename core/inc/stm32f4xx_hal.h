/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Stub for CI/QEMU Verification.
 */

#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RNG             ((void*)0x40026000)
#define HAL_OK          0x00

typedef struct {
    void* Instance;
} RNG_HandleTypeDef;

/* HAL Core */
void HAL_Init(void);
void SystemClock_Config(void);
uint32_t HAL_GetTick(void);

/* RNG */
void HAL_RNG_Init(RNG_HandleTypeDef* hrng);
void HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* hrng, uint32_t* random32);
void __HAL_RNG_ENABLE(RNG_HandleTypeDef* hrng);
void __HAL_RNG_DISABLE(RNG_HandleTypeDef* hrng);

/* IWDG */
typedef struct { void* Instance; } IWDG_HandleTypeDef;
void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg);

/* Mock UART/Printf support */
#include <stdio.h>

#endif /* STM32F4XX_HAL_H */
