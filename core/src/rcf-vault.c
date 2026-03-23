/* 
 * [RCF:NOTICE][RCF:RESTRICTED]
 * RCF-PL v1.2.7 — Restricted Correlation Framework
 * Secure Key Vault & Anti-Tamper Storage.
 */

#include "rcf_vault.h"
#include "rcf_crypto.h"
#include "rcf_bunker.h"
#include "rcf_pilloff.h"
#include "stm32f4xx_hal.h"

// Vault layout in Sector 11 (128KB):
// [0x0000-0x00FF]: Header (magic, version, checksum)
// [0x0100-0x04FF]: Key table (4 keys × 256 bytes)
// [0x0500-0x1FFF]: Reserved / Wear leveling
// [0x2000-0xFFFF]: Encrypted backup copies

#define VAULT_HEADER_ADDR       RCF_VAULT_ADDR
#define VAULT_KEYTABLE_ADDR     (RCF_VAULT_ADDR + 0x100)
#define VAULT_MAGIC             0x56414D54  // "VAMT"

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
    uint8_t  device_uid[12];     // STM32 unique ID
    uint8_t  header_hash[32];    // SHA-256 of header
    uint32_t key_count;
    uint32_t generation;         // For anti-rollback
} Vault_Header;

static Vault_Header vault_header;
static bool vault_initialized = false;

bool vault_init(void) {
    // Read header
    memcpy(&vault_header, (void*)VAULT_HEADER_ADDR, sizeof(Vault_Header));
    
    // Verify magic
    if (vault_header.magic != VAULT_MAGIC) {
        // Uninitialized vault — enter provisioning mode
        return false;
    }
    
    // Verify header integrity
    uint8_t computed_hash[32];
    sha256_hw((uint8_t*)&vault_header, offsetof(Vault_Header, header_hash), computed_hash);
    if (memcmp(computed_hash, vault_header.header_hash, 32) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_VAULT);
        return false;
    }
    
    // Verify device UID binding
    uint8_t chip_uid[12];
    HAL_GetUID(chip_uid);
    if (memcmp(chip_uid, vault_header.device_uid, 12) != 0) {
        // Vault cloned to different chip
        trigger_pill_off(PILL_OFF_TAMPER_CLONE);
        return false;
    }
    
    vault_initialized = true;
    return true;
}

bool vault_load_key(Vault_KeyType type, uint8_t* out_buffer, uint32_t* out_len) {
    if (!vault_initialized || type >= VAULT_KEY_COUNT) {
        return false;
    }
    
    // Calculate key entry address
    uint32_t entry_addr = VAULT_KEYTABLE_ADDR + (type * 256);
    Vault_KeyEntry entry;
    memcpy(&entry, (void*)entry_addr, sizeof(Vault_KeyEntry));
    
    // Verify key integrity
    uint8_t computed_checksum[32];
    sha256_hw(entry.key_data, entry.key_data[0] + 1, computed_checksum); // key_data[0] = actual length
    if (memcmp(computed_checksum, entry.key_checksum, 32) != 0) {
        trigger_pill_off(PILL_OFF_TAMPER_KEY);
        return false;
    }
    
    // Copy to output (still in encrypted form — decryption happens in crypto module)
    memcpy(out_buffer, entry.key_data, entry.key_data[0]);
    *out_len = entry.key_data[0];
    // [RCF-END]
    
    return true;
}

void vault_zeroize_all(void) {
    // [RCF-START][M-VAULT-ZEROIZE]
    // Multi-pass overwrite before erase
    FLASH_EraseInitTypeDef erase;
    uint32_t error = 0;
    
    __disable_irq();
    
    // Pass 1: Overwrite with pattern
    volatile uint32_t* ptr = (uint32_t*)RCF_VAULT_ADDR;
    for (int i = 0; i < RCF_VAULT_SIZE / 4; i++) {
        ptr[i] = 0xAAAAAAAA;
    }
    
    // Pass 2: Complement pattern
    for (int i = 0; i < RCF_VAULT_SIZE / 4; i++) {
        ptr[i] = 0x55555555;
    }
    
    // Pass 3: Random data from TRNG
    for (int i = 0; i < RCF_VAULT_SIZE / 4; i++) {
        uint32_t random;
        HAL_RNG_GenerateRandomNumber(&hrng, &random);
        ptr[i] = random;
    }
    // [RCF-END]
    
    // Final erase
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = RCF_VAULT_SECTOR;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&erase, &error);
    HAL_FLASH_Lock();
    
    vault_initialized = false;
    
    // Clear any key material in SRAM
    secure_memzero(&vault_header, sizeof(vault_header));
}

void rcf_vault_emergency_shutdown(void) {
    // [RCF-START][M-VAULT-EMERGENCY]
    // 1. Immediate RAM wipe (0.01 ms)
    secure_memzero(&vault_header, sizeof(vault_header));
    
    // 2. Kill TRNG to protect against voltage-based leakage
    __HAL_RNG_DISABLE(&hrng);

    // 3. Destruct Backup SRAM (Timechain & Key Pointers)
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    memset((void*)BKPSRAM_BASE, 0x00, 4096); 
    
    // 4. Force Isolation (Disconnect interfaces)
    __disable_irq(); 
    
    // 5. High-speed Flash Corruption (Overwrite Header)
    // Faster than erase, kills integrity before power loss
    HAL_FLASH_Unlock();
    for(int i=0; i<32; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, VAULT_HEADER_ADDR + (i*4), 0x00000000);
    }
    HAL_FLASH_Lock();
    // [RCF-END]
}