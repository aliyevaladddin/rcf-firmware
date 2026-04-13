/* 
 * [RCF:PROTECTED]
 * hal_pka.h — Sentinel PKA Peripheral Definitions (Mil-Spec RC2-LS)
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#ifndef HAL_PKA_H
#define HAL_PKA_H

#include <stdint.h>
#include <stdbool.h>

/* Sentinel PKA Base Address (MMIO Space) */
#define PKA_BASE_ADDR       0x50000000U

/* Register Offsets (Aladdin's Spec v1.0) */
#define PKA_REG_CTRL        0x00U  /* CR: Control Register */
#define PKA_REG_STAT        0x04U  /* SR: Status Register */
#define PKA_REG_CMD         0x08U  /* CMD: Command Register */
#define PKA_REG_DATA_IN0    0x10U  /* DATA_IN0: SIG Pointer */
#define PKA_REG_DATA_IN1    0x14U  /* DATA_IN1: MSG Pointer */
#define PKA_REG_DATA_IN2    0x18U  /* DATA_IN2: PK Pointer */
#define PKA_REG_LEN_MSG     0x1CU  /* LEN: Message Length */
#define PKA_REG_RESULT      0x20U  /* RESULT: DATA_OUT */

/* Control Bits */
#define PKA_CTRL_START      (1U << 0)
#define PKA_CTRL_RESET      (1U << 1)

/* Status Bits */
#define PKA_STAT_BUSY       (1U << 0)
#define PKA_STAT_DONE       (1U << 1)
#define PKA_STAT_ERROR      (1U << 2)

/* Commands */
#define PKA_CMD_DILITHIUM2_VERIFY  0x01U

/* --- Mil-Spec Hardware Interface --- */

/**
 * @brief Initialize the PKA Peripheral.
 */
void hal_pka_init(void);

/**
 * @brief Load data pointer into specific PKA register.
 */
void hal_pka_load_data(uint32_t reg_offset, const uint8_t* data, uint32_t len);

/**
 * @brief Start PKA hardware processing.
 */
void hal_pka_start(void);

/**
 * @brief Check if hardware has finished the operation.
 * @return true if DONE, false if BUSY.
 */
bool hal_pka_is_ready(void);

/**
 * @brief Read the final result from PKA result register.
 */
void hal_pka_read_result(uint32_t* out_result);

/* High-level driver for PQC layer */
int hal_pka_verify_dilithium2(const uint8_t* sig, const uint8_t* msg, uint32_t len, const uint8_t* pk);

#endif /* HAL_PKA_H */
