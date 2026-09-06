APP ?= bare-blink
PROJECT := bluepill-$(APP)

BUILD_DIR := build
SRC_DIR := src
APP_DIR := apps/$(APP)
STARTUP_DIR := startup
LINKER_SCRIPT := linker/STM32F103C8TX_FLASH.ld
OPENOCD_CFG := openocd/bluepill.cfg

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size
GDB := arm-none-eabi-gdb
OPENOCD := openocd

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
COMMON_FLAGS := $(CPU_FLAGS) -Wall -Wextra -Werror -ffunction-sections -fdata-sections -g3 -O0
CFLAGS := $(COMMON_FLAGS) -std=c11 -Iinclude
ASFLAGS := $(COMMON_FLAGS) -x assembler-with-cpp
LDFLAGS := $(CPU_FLAGS) -T$(LINKER_SCRIPT) -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map --specs=nano.specs --specs=nosys.specs

C_SOURCES := $(wildcard $(SRC_DIR)/*.c)
C_SOURCES += $(wildcard $(APP_DIR)/*.c)
ASM_SOURCES :=

ifeq ($(wildcard $(APP_DIR)),)
$(error Unknown APP '$(APP)': expected directory $(APP_DIR))
endif

OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
OBJECTS += $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

ELF := $(BUILD_DIR)/$(PROJECT).elf
BIN := $(BUILD_DIR)/$(PROJECT).bin
HEX := $(BUILD_DIR)/$(PROJECT).hex

.PHONY: all clean flash openocd debug size

all: $(ELF) $(BIN) $(HEX) size

$(ELF): $(OBJECTS) $(LINKER_SCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

size: $(ELF)
	$(SIZE) $<

flash: $(ELF)
	$(OPENOCD) -f $(OPENOCD_CFG) -c "program $(ELF) verify reset exit"

openocd:
	$(OPENOCD) -f $(OPENOCD_CFG)

debug: $(ELF)
	$(GDB) $(ELF) -ex "target extended-remote localhost:3333" -ex "monitor reset halt"

clean:
	rm -rf $(BUILD_DIR)
