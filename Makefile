# Toolchain
CXX = g++
LD = ld

# Host detection
UNAME_S := $(shell uname -s)

# Assembler setup
AS = as
ASFLAGS = --64

ifeq ($(UNAME_S),Darwin)
AS = clang
ASFLAGS = -c -arch x86_64
endif

# Flags
# CXXFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
#            -nostdlib -nostdinc -nostdinc++ -mno-red-zone -mcmodel=kernel \
# 					 -fcf-protection=none -mno-sse -mno-sse2 \
#            -fno-stack-protector -fno-pic -Iinclude -g

CXXFLAGS = -O2 -Wall -Wextra -fno-exceptions -ffreestanding -fno-builtin \
				   -fno-builtin-memset -fno-builtin-memcpy \
					 -fcf-protection=none  -fno-pic \
					 -fno-stack-protector  -fno-rtti -nostdlib -nostartfiles \
					 -mno-red-zone -mcmodel=kernel -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
					 -fno-tree-vectorize -fno-tree-slp-vectorize -Iinclude -g

LDFLAGS = -n -T linker.ld -nostdlib

# Directories
SRC_DIR = src
BOOT_DIR = boot
BUILD_DIR = build
ISO_DIR = isodir

# Source files
ASM_SOURCES = $(wildcard $(BOOT_DIR)/*.S)
CXX_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
ASM_OBJECTS = $(patsubst $(BOOT_DIR)/%.S, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
CXX_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CXX_SOURCES))
OBJECTS = $(ASM_OBJECTS) $(CXX_OBJECTS)

# Output
KERNEL_ELF = $(BUILD_DIR)/hypervisor.elf
KERNEL_BIN = $(BUILD_DIR)/hypervisor.bin
ISO = hypervisor.iso

.PHONY: all clean run iso dirs

all: dirs $(KERNEL_BIN)

dirs:
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.S | dirs
	@echo "AS    $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | dirs
	@echo "CXX   $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJECTS)
	@echo "LD    $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "Build complete: $(KERNEL_ELF)"

$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "OBJCOPY $@"
	@objcopy -O binary $< $@
	@echo "Binary created: $(KERNEL_BIN)"


iso: $(KERNEL_ELF)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/hypervisor.elf
	@echo 'set timeout=5' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "Hypervisor" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/hypervisor.elf' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "Hypervisor Debug" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    echo "Loading hypervisor..."' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/hypervisor.elf' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    echo "Booting..."' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null
	@echo "ISO created: $(ISO)"

run: iso
	@echo "Starting QEMU..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-boot d  -d in_asm,int,cpu_reset,guest_errors -nographic -D qemu.log

debug-iso: iso
	@echo "Starting QEMU with GDB support..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-boot d -s -S -nographic -D qemu.log

debug: $(KERNEL_ELF)
	@echo "Starting QEMU with GDB support..."
	@qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512M -cpu host -enable-kvm  -nographic
clean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO) *.log *.bin
	@echo "Clean complete"

info:
	@echo "ASM Sources: $(ASM_SOURCES)"
	@echo "CXX Sources: $(CXX_SOURCES)"
	@echo "Objects:     $(OBJECTS)"
	@echo "Kernel:      $(KERNEL)"
