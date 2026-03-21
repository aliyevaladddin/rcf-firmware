# [RCF:PUBLIC] — Build configuration
TARGET = rcf-lume-firmware
MCU = STM32F407VG

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

# Flags
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -O2 -g -Wall -Wextra
CFLAGS += -DUSE_HAL_DRIVER -DSTM32F407xx
CFLAGS += -DRCF_RESTRICTED_ACCESS  # Enable [RCF:RESTRICTED] sections

# Security flags
CFLAGS += -fstack-protector-strong  # Stack protection
CFLAGS += -fPIE -fPIC               # Position independent
CFLAGS += -fno-inline-functions     # Prevent optimization of crypto
CFLAGS += -D_FORTIFY_SOURCE=2       # Buffer overflow detection

# Linker flags
LDFLAGS = -TSTM32F407VGTx_FLASH.ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(TARGET).map

# Source files
SRCS = \
    Core/Src/main.c \
    Core/Src/system_stm32f4xx.c \
    Core/Src/rcf_vault.c \
    Core/Src/rcf_timechain.c \
    Core/Src/rcf_license.c \
    Core/Src/rcf_crypto.c \
    Core/Src/rcf_protocol.c \
    Core/Src/rcf_audit.c \
    Core/Src/rcf_pulse.c \
    Core/Src/rcf_led.c \
    Core/Src/rcf_pilloff.c \
    # ... HAL sources ...

# Build rules
.PHONY: all clean provision

all: $(TARGET).elf $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $@ $(TARGET).bin

# Provisioning tool (requires HSM)
provision:
	python3 Build/rcf_provision.py --device $(TARGET).bin --hsm yubihsm

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map