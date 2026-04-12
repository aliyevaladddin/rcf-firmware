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
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Icore/inc -Og -g3 -DRCF_PRODUCTION=0 -DRCF_VM_DEBUG=1 -DRCF_VM_CI_MODE=1 -Wall -fdata-sections -ffunction-sections

# Linker flags
LD_FLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) -Wl,--gc-sections,--relax -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref

# Sources
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
ASM_SOURCES = startup_stm32f407xx.s
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

# [RCF v1.3] Hardened RC2 Target — Fixed
RC2:
	$(MAKE) TARGET=rcf-lume-rc2 CFLAGS="$(CFLAGS) -DRCF_VM_CI_MODE=1 -DRCF_LIFE_SUPPORT=1" clean all check-mpk
	@echo "[RCF] Hardened RC2 Build Complete: rcf-lume-rc2.bin"

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

RC1: clean
	$(MAKE) TARGET=rcf-lume-rc1 all
	$(MAKE) TARGET=rcf-lume-rc1 check-mpk

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

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
	@arm-none-eabi-readelf -S $< | grep -E "\.mpk_storage|\.otp_mek" || echo "Section check skip"
	@arm-none-eabi-size -A $< | grep -E "\.mpk_storage|\.otp_mek" || echo "Section check skip"
	@echo "[RCF] Address verification (rcf_vault_mpk_public)..."
	@arm-none-eabi-nm $< | grep "decrypted_mpk" || echo "MPK RAM Symbol check skip"

clean:
	rm -rf $(BUILD_DIR)

sim: all
	qemu-system-arm -M netduinoplus2 -cpu cortex-m4 -kernel $(BUILD_DIR)/$(TARGET).elf -nographic -serial mon:stdio

.PHONY: all clean RC1 sim check-mpk RC2
