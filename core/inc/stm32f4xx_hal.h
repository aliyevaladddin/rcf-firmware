/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Minimal HAL Stub for CI/QEMU Verification.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RNG             ((void*)0x40026000)
#define RTC             ((RTC_TypeDef*)0x40002800)
#define TIM5            ((TIM_TypeDef*)0x40000C00)

#define HAL_OK          0x00
#define RTC_FORMAT_BIN  0x00000000U

/* Forward declarations */
typedef struct { 
    uint32_t TR;
    uint32_t DR;
    uint32_t CR;
    uint32_t ISR;
    uint32_t PRER;
    uint32_t WUTR;
    uint32_t CALIBR;
    uint32_t ALRMAR;
    uint32_t ALRMBR;
    uint32_t WPR;
    uint32_t SSR;     /* Subsecond register used in timechain */
    uint32_t SHIFTR;
} RTC_TypeDef;

typedef struct { 
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SMCR;
    uint32_t DIER;
    uint32_t SR;
    uint32_t EGR;
    uint32_t CCMR1;
    uint32_t CCMR2;   /* Used in timechain */
    uint32_t CCER;    /* Used in timechain */
    uint32_t CNT;
    uint32_t PSC;
    uint32_t ARR;
    uint32_t RCR;
    uint32_t CCR1;
    uint32_t CCR2;
    uint32_t CCR3;
    uint32_t CCR4;    /* Capture/Compare register used in timechain */
} TIM_TypeDef;

#define TIM_CCER_CC4E       (1U << 12)
#define TIM_CCMR2_CC4S_0    (1U << 8)

typedef struct {
    void* Instance;
} RNG_HandleTypeDef;

typedef struct {
    RTC_TypeDef* Instance;
} RTC_HandleTypeDef;

typedef struct {
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
} RTC_TimeTypeDef;

typedef struct {
    uint8_t WeekDay;
    uint8_t Month;
    uint8_t Date;
    uint8_t Year;
} RTC_DateTypeDef;

/* HAL Core */
void HAL_Init(void);
void SystemClock_Config(void);
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t Delay);

/* Unique ID Access */
static inline uint32_t HAL_GetUIDw0(void) { return 0x12345678; }
static inline uint32_t HAL_GetUIDw1(void) { return 0xABCDEF01; }
static inline uint32_t HAL_GetUIDw2(void) { return 0xBADC0FFE; }

/* RNG */
void HAL_RNG_Init(RNG_HandleTypeDef* phrng);
void HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef* phrng, uint32_t* random32);
void __HAL_RNG_ENABLE(RNG_HandleTypeDef* phrng) { (void)phrng; }
void __HAL_RNG_DISABLE(RNG_HandleTypeDef* phrng) { (void)phrng; }

/* IWDG */
typedef struct { void* Instance; } IWDG_HandleTypeDef;
void HAL_IWDG_Refresh(IWDG_HandleTypeDef* hiwdg);

/* RTC */
void HAL_RTC_GetTime(RTC_HandleTypeDef* phrtc, RTC_TimeTypeDef* sTime, uint32_t Format);
void HAL_RTC_GetDate(RTC_HandleTypeDef* phrtc, RTC_DateTypeDef* sDate, uint32_t Format);

/* Clock macros (CI stubs) */
#define __HAL_RCC_BKPSRAM_CLK_ENABLE()  ((void)0)
#define __HAL_RCC_RTC_ENABLE()          ((void)0)
#define __HAL_RTC_IS_INITIALIZED(hrtc)  (true)

/* Mock UART/Printf support */
#include <stdio.h>

#endif /* STM32F4XX_HAL_H */
