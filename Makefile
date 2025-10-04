# Toolchain
AS = as
CXX = g++
LD = ld

# Flags
# ASFLAGS = --32
CXXFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
           -nostdlib -nostdinc -nostdinc++ -mno-red-zone -mcmodel=kernel \
           -fno-stack-protector -fno-pic -Iinclude -g
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
	@mkdir -p $(ISO_DIR)/boot
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/hypervisor.elf
	@echo 'set timeout=0' > $(ISO_DIR)/boot/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub.cfg
	@echo 'menuentry "Hypervisor" {' >> $(ISO_DIR)/boot/grub.cfg
	@echo '    multiboot2 /boot/hypervisor.elf' >> $(ISO_DIR)/boot/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null
	@echo "ISO created: $(ISO)"

run: iso
	@echo "Starting QEMU..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-boot d -d int,cpu_reset -no-reboot -no-shutdown -nographic 

debug-iso: iso
	@echo "Starting QEMU with GDB support..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-boot d -s -S -nographic

debug: $(KERNEL_ELF)
	@echo "Starting QEMU with GDB support..."
	@qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512M -cpu host -enable-kvm  -nographic
clean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO) *.log
	@echo "Clean complete"

info:
	@echo "ASM Sources: $(ASM_SOURCES)"
	@echo "CXX Sources: $(CXX_SOURCES)"
	@echo "Objects:     $(OBJECTS)"
	@echo "Kernel:      $(KERNEL)"
