# Toolchain
AS = as
CXX = g++
LD = ld

# Flags
ASFLAGS = --64
CXXFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti \
           -nostdlib -nostdinc -nostdinc++ -mno-red-zone -mcmodel=kernel \
           -fno-stack-protector -fno-pic -Iinclude
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
KERNEL = $(BUILD_DIR)/hypervisor.elf
ISO = hypervisor.iso

.PHONY: all clean run iso dirs

all: dirs $(KERNEL)

dirs:
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.S
	@echo "AS    $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "CXX   $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(KERNEL): $(OBJECTS)
	@echo "LD    $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "Build complete: $(KERNEL)"

iso: $(KERNEL)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL) $(ISO_DIR)/boot/
	@echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "Hypervisor" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/hypervisor.elf' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null
	@echo "ISO created: $(ISO)"

run: iso
	@echo "Starting QEMU..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-serial stdio -d int,cpu_reset -no-reboot -no-shutdown 

debug: iso
	@echo "Starting QEMU with GDB support..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu host -enable-kvm \
		-serial stdio -s -S

clean:
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)
	@echo "Clean complete"

info:
	@echo "ASM Sources: $(ASM_SOURCES)"
	@echo "CXX Sources: $(CXX_SOURCES)"
	@echo "Objects:     $(OBJECTS)"
	@echo "Kernel:      $(KERNEL)"
