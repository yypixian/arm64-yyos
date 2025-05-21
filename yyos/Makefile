ARMGNU ?= aarch64-linux-gnu

board ?= rpi4

ifeq ($(board), rpi3)
COPS += -DCONFIG_BOARD_PI3B
QEMU_FLAGS  += -machine raspi3
else ifeq ($(board), rpi4)
COPS += -DCONFIG_BOARD_PI4B
QEMU_FLAGS  += -machine raspi4
endif

COPS += -g -Wall -nostdlib -Iinclude -mgeneral-regs-only


ASMOPS = -g -Iinclude 


BUILD_ROOT= build
BUILD_DIR = $(BUILD_ROOT)/benos

LIB_DIR = lib
BUILD_LIB_DIR = $(BUILD_ROOT)/lib
SRC_DIR = src

all : benos.bin

clean :
	rm -rf $(BUILD_DIR) *.bin 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_LIB_DIR)/%_c.o: $(LIB_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
LIB_FILES = $(wildcard $(LIB_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)


OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(LIB_FILES:$(LIB_DIR)/%.c=$(BUILD_LIB_DIR)/%_c.o)


DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)



#表示编写一个.bin的文件需要依赖于链接脚本和所有的.o文件
#-ld表示使用编译器的链接脚本，-T指明使用哪个脚本，-o表示链接完成之后的输出文件
#objcopy 是 GNU Binutils 中的一个工具，用于 转换目标文件格式,-O 表示输出格式（--output-target）
benos.bin: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -Map benos.map -o $(BUILD_DIR)/benos.elf  $(OBJ_FILES) -e _start
	$(ARMGNU)-objcopy $(BUILD_DIR)/benos.elf -O binary benos.bin

QEMU_FLAGS  += -nographic

run:
	qemu-system-aarch64 $(QEMU_FLAGS) -kernel benos.bin
debug:
	qemu-system-aarch64 $(QEMU_FLAGS) -kernel benos.bin -S -s

