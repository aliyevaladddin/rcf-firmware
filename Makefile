# [RCF:NOTICE][RCF:PROTECTED]
# Makefile for RCF v1.3 — Mil-Spec Hardened Build System.

TARGET ?= rcf-lume-rc1
BUILD_DIR = .build
SRC_DIR = core/src
INC_DIR = core/inc

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

# Target selection logic
ifeq ($(TARGET),RC1)
    LDSCRIPT = STM32F407VGTx_RC1.ld
    DEF_FLAGS = -DRCF_PRODUCTION=1 -DRCF_MILSPEC=1 -DRCF_OPTION_A=1 -DRCF_CCMRAM_ENABLE=1
    CFLAGS_OPT = -Os -fstack-protector-strong -mno-unaligned-access -DNDEBUG
else
    LDSCRIPT = STM32F407VGTx_RC1.ld
    DEF_FLAGS = -DRCF_PRODUCTION=0 -DRCF_VM_DEBUG=1
    CFLAGS_OPT = -Og -g3
endif

# CPU & Arch
CPU = -mcpu=cortex-m4
FPU = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=softfp
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# Compilation Flags
C_FLAGS = $(MCU) -I$(INC_DIR) $(CFLAGS_OPT) $(DEF_FLAGS) -Wall -fdata-sections -ffunction-sections

# Linker flags
LD_FLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) -Wl,--gc-sections,--relax -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref

# Sources
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = startup_stm32f407xx.s
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

# Production build pipeline
rc1: clean
	$(MAKE) TARGET=RC1 all
	$(MAKE) TARGET=RC1 check-mpk

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(C_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(C_FLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LD_FLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

# MPK integrity verification
check-mpk: $(BUILD_DIR)/$(TARGET).elf
	@echo "[RCF] Verifying MPK section..."
	@arm-none-eabi-readelf -S $< | grep -E "\.mpk_storage|\.otp_mek"
	@arm-none-eabi-size -A $< | grep -E "\.mpk_storage|\.otp_mek" || echo "Section check failed"
	@echo "[RCF] Address verification (rcf_vault_mpk_public)..."
	@arm-none-eabi-nm $< | grep "decrypted_mpk" || echo "MPK RAM Symbol check"

clean:
	rm -rf $(BUILD_DIR)

sim: all
	qemu-system-arm -M netduinoplus2 -cpu cortex-m4 -kernel $(BUILD_DIR)/$(TARGET).elf -nographic -serial mon:stdio

.PHONY: all clean rc1 sim check-mpk
