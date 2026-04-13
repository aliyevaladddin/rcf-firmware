/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * NOTICE: This file is protected under RCF-PL v1.2.8
 * Post-Quantum Cryptography (PQC) Verification Layer.
 */

#include "rcf_crypto.h"
#include "hal_pka.h"
#include "rcf_config.h"

int rcf_pqc_verify(const uint8_t* sig, const uint8_t* msg, int len, const uint8_t* pk) {
    // [RCF-START][M-PQC-VERIFY]
    // Загрузка в PKA регистры (Aladdin Spec RC2-LS)
    hal_pka_load_data(PKA_REG_DATA_IN0, sig, RCF_PQC_SIG_SIZE);
    hal_pka_load_data(PKA_REG_DATA_IN1, msg, (uint32_t)len);
    hal_pka_load_data(PKA_REG_DATA_IN2, pk, RCF_PQC_PUBKEY_SIZE);
    hal_pka_load_data(PKA_REG_LEN_MSG,  NULL, (uint32_t)len);
    
    // Запуск «железа»
    hal_pka_start();
    
    // Ожидание (как настоящий polling)
    while (!hal_pka_is_ready()) {
        __asm("nop"); 
    }
    
    // Чтение результата
    uint32_t result;
    hal_pka_read_result(&result);
    return (result == 0) ? 0 : -1; // 0 = valid signature
    // [RCF-END]
}
