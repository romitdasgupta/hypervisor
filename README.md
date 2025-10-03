# Type-1 Hypervisor - Phase 1

A minimal x86-64 bare-metal hypervisor with Intel VT-x support, written in C++ and assembly.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Setting Up Your Build Machine](#setting-up-your-build-machine)
- [Project Setup](#project-setup)
- [Building the Hypervisor](#building-the-hypervisor)
- [Running the Hypervisor](#running-the-hypervisor)
- [Expected Output](#expected-output)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [What Phase 1 Accomplishes](#what-phase-1-accomplishes)
- [Next Steps](#next-steps)

---

## Prerequisites

### Hardware Requirements
- x86-64 CPU with Intel VT-x support (or AMD-V for future phases)
- At least 2GB RAM
- 500MB free disk space

### Software Requirements
- Linux operating system (Ubuntu, Fedora, Arch, or similar)
- Internet connection for downloading packages

---

## Setting Up Your Build Machine

### Ubuntu / Debian / Linux Mint

```bash
# Update package lists
sudo apt-get update

# Install required packages
sudo apt-get install -y \
    build-essential \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    qemu-kvm \
    make

# Verify installations
nasm --version        # Should show NASM version
g++ --version         # Should show GCC version
qemu-system-x86_64 --version  # Should show QEMU version
grub-mkrescue --version       # Should show GRUB version
```

### Fedora / RHEL / CentOS

```bash
# Install required packages
sudo dnf install -y \
    gcc \
    gcc-c++ \
    make \
    nasm \
    grub2-tools \
    grub2-pc-modules \
    xorriso \
    qemu-system-x86 \
    qemu-kvm

# Verify installations
nasm --version
g++ --version
qemu-system-x86_64 --version
grub2-mkrescue --version
```

### Arch Linux / Manjaro

```bash
# Install required packages
sudo pacman -S --needed \
    base-devel \
    nasm \
    grub \
    xorriso \
    qemu-system-x86 \
    qemu-desktop

# Verify installations
nasm --version
g++ --version
qemu-system-x86_64 --version
grub-mkrescue --version
```

### Verifying KVM Support (Optional but Recommended)

KVM provides hardware acceleration for QEMU, making testing much faster:

```bash
# Check if your CPU supports virtualization
egrep -c '(vmx|svm)' /proc/cpuinfo

# If the output is > 0, your CPU supports virtualization
# vmx = Intel VT-x, svm = AMD-V

# Check if KVM modules are loaded
lsmod | grep kvm

# If not loaded, load them:
sudo modprobe kvm
sudo modprobe kvm_intel  # For Intel CPUs
# OR
sudo modprobe kvm_amd    # For AMD CPUs

# Add your user to the kvm group (requires logout/login to take effect)
sudo usermod -aG kvm $USER
```

---

## Project Setup

### 1. Create Project Directory Structure

```bash
# Create main project directory
mkdir -p ~/hypervisor
cd ~/hypervisor

# Create subdirectories
mkdir -p boot src include build
```

### 2. Create All Source Files

Create the following files with the content provided in the code artifacts:

**Directory Structure:**
```
hypervisor/
├── boot/
│   └── boot.S              # Assembly bootloader
├── src/
│   ├── kernel.cpp          # Main kernel entry point
│   ├── vga.cpp             # VGA text mode driver
│   └── cpuid.cpp           # CPU feature detection
├── include/
│   ├── types.h             # Basic type definitions
│   ├── vga.h               # VGA driver header
│   ├── cpuid.h             # CPUID header
│   └── vmx.h               # VMX header (placeholder)
├── linker.ld               # Linker script
├── Makefile                # Build system
└── README.md               # This file
```

**Copy each file from the artifacts provided earlier, or use this quick script:**

```bash
# This assumes you have all the code files ready
# Place them in their respective directories as shown above
```

### 3. Verify File Permissions

```bash
# Ensure all files are readable
chmod 644 boot/* src/* include/* linker.ld Makefile

# Verify structure
tree .
# Or if tree is not installed:
find . -type f
```

---

## Building the Hypervisor

### Basic Build

```bash
# From the hypervisor directory
cd ~/hypervisor

# Build the kernel
make

# Expected output:
# AS    boot/boot.S
# CXX   src/cpuid.cpp
# CXX   src/kernel.cpp
# CXX   src/vga.cpp
# LD    build/hypervisor.elf
# Build complete: build/hypervisor.elf
```

### Build Bootable ISO

```bash
# Create a bootable ISO image
make iso

# Expected output:
# ... (build steps as above)
# ISO created: hypervisor.iso
```

### Clean Build

```bash
# Remove all build artifacts
make clean

# Rebuild from scratch
make iso
```

### Build Targets

- `make` or `make all` - Build the kernel ELF file
- `make iso` - Build kernel and create bootable ISO
- `make run` - Build, create ISO, and run in QEMU
- `make debug` - Build, create ISO, and run in QEMU with GDB support
- `make clean` - Remove all build artifacts
- `make info` - Display build configuration

---

## Running the Hypervisor

### Method 1: Using Make (Recommended)

```bash
# Build and run in one command
make run
```

This will:
1. Build the kernel
2. Create bootable ISO
3. Launch QEMU with KVM acceleration
4. Display output in the terminal

### Method 2: Manual QEMU Execution

```bash
# First, build the ISO
make iso

# Then run with QEMU
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm
```

### Method 3: Without KVM (if KVM is unavailable)

If you don't have KVM support or are running in a VM:

```bash
# Edit Makefile, change the run target to:
run: iso
	@echo "Starting QEMU..."
	@qemu-system-x86_64 -cdrom $(ISO) -m 512M -cpu qemu64,+vmx

# Then run:
make run
```

### Running with Additional Options

```bash
# Run with serial output
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm -serial stdio

# Run with VNC display (accessible at localhost:5900)
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm -vnc :0

# Run with more memory
qemu-system-x86_64 -cdrom hypervisor.iso -m 2G -cpu host -enable-kvm
```

### Debugging with GDB

**Terminal 1:**
```bash
# Start QEMU in debug mode (waits for GDB)
make debug
```

**Terminal 2:**
```bash
# Connect GDB
gdb build/hypervisor.elf

# Inside GDB:
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
(gdb) layout src        # View source code
(gdb) info registers    # View CPU registers
(gdb) x/10i $rip        # Disassemble at instruction pointer
```

---

## Expected Output

When you run `make run`, you should see:

```
================================
  Type-1 Hypervisor v0.1
================================

Multiboot Magic: 0x0000000036D76289
Multiboot OK

CPU Vendor: GenuineIntel
Long Mode: Yes
VMX Support: Yes
MSR Support: Yes

Hypervisor initialization complete!
Ready for VMX setup in next phase.
```

**Color Coding:**
- Green: Success messages
- Cyan: Information labels
- White: Data values
- Red: Errors (if any)

---

## Troubleshooting

### Build Errors

**Error: `nasm: command not found`**
```bash
# Install NASM
sudo apt-get install nasm  # Ubuntu/Debian
sudo dnf install nasm      # Fedora
```

**Error: `grub-mkrescue: command not found`**
```bash
# Install GRUB tools
sudo apt-get install grub-pc-bin grub-common xorriso  # Ubuntu/Debian
sudo dnf install grub2-tools xorriso                   # Fedora
```

**Error: Linker errors about undefined references**
```bash
# Make sure all source files are present and in correct directories
ls -la boot/ src/ include/

# Clean and rebuild
make clean
make iso
```

### Runtime Errors

**Error: `VMX Support: No`**

Your CPU doesn't have VT-x enabled or doesn't support it.

```bash
# Check CPU support
grep vmx /proc/cpuinfo

# If empty, either:
# 1. Your CPU doesn't support VT-x
# 2. VT-x is disabled in BIOS/UEFI

# To enable: Reboot → Enter BIOS → Find "Intel Virtualization Technology" → Enable
```

**Error: `Could not access KVM kernel module`**

```bash
# Load KVM modules
sudo modprobe kvm
sudo modprobe kvm_intel

# Add user to kvm group
sudo usermod -aG kvm $USER

# Log out and log back in, or run:
newgrp kvm

# Alternatively, run QEMU without KVM (slower):
# Edit Makefile and remove -enable-kvm
```

**Error: QEMU crashes or displays garbled output**

```bash
# Try without KVM
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu qemu64

# Try older QEMU version or update to latest
sudo apt-get update
sudo apt-get upgrade qemu-system-x86
```

**Error: Black screen with no output**

```bash
# Check if ISO was created properly
ls -lh hypervisor.iso

# Try running with more verbose output
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm -d int

# Check QEMU log
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -D qemu.log -d int,cpu_reset
cat qemu.log
```

### Common Issues

**Multiboot Magic is Wrong**

The displayed magic should be `0x36d76289`. If different, GRUB didn't load properly.

**VMX Not Detected in QEMU**

```bash
# Make sure to use -cpu host or -cpu with +vmx
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm

# Or explicitly enable VMX:
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu qemu64,+vmx
```

---

## Project Structure

```
hypervisor/
├── boot/
│   └── boot.S              # Multiboot2 header + 32→64 bit transition
├── src/
│   ├── kernel.cpp          # Main kernel entry, C++ runtime
│   ├── vga.cpp             # VGA text mode output driver
│   └── cpuid.cpp           # CPUID wrapper and feature detection
├── include/
│   ├── types.h             # Basic integer types
│   ├── vga.h               # VGA driver interface
│   ├── cpuid.h             # CPUID interface
│   └── vmx.h               # VMX constants (Phase 2 placeholder)
├── linker.ld               # Custom linker script (1MB load address)
├── Makefile                # Build system
├── README.md               # This file
├── build/                  # Generated build artifacts (created by make)
│   ├── boot.o
│   ├── kernel.o
│   ├── vga.o
│   ├── cpuid.o
│   └── hypervisor.elf
└── hypervisor.iso          # Bootable ISO image (created by make iso)
```

---

## What Phase 1 Accomplishes

✅ **Bare Metal Boot**
- Multiboot2 compliant bootloader
- Loads via GRUB2

✅ **64-bit Long Mode**
- Transitions from 32-bit protected mode to 64-bit long mode
- Sets up paging (identity-mapped first 1GB using 2MB pages)

✅ **Basic I/O**
- VGA text mode driver with color support
- Screen scrolling
- Hex value printing

✅ **CPU Feature Detection**
- CPUID wrapper
- Detects VMX (Intel VT-x) support
- Detects MSR support
- Displays CPU vendor string

✅ **Clean C++ Architecture**
- Minimal freestanding C++ runtime
- No standard library dependencies
- Modular design ready for expansion

---

## Next Steps

### Phase 2: VMX Initialization (Upcoming)

1. **MSR Operations**
   - Implement read_msr() and write_msr()
   - Check IA32_FEATURE_CONTROL MSR

2. **Enable VMX**
   - Set CR4.VMXE bit
   - Allocate VMXON region (4KB aligned)
   - Execute VMXON instruction

3. **VMCS Setup**
   - Allocate VMCS region
   - Initialize VMCS fields
   - Configure guest state

4. **VM Entry/Exit**
   - Set up basic VM execution controls
   - Handle VM exits
   - Test simple guest code

### Phase 3: Guest Management

1. Memory virtualization (EPT)
2. Interrupt handling
3. I/O port emulation
4. Multiple VCPU support

### Phase 4: Advanced Features

1. Device passthrough
2. Nested paging optimization
3. Performance monitoring
4. Save/restore VM state

---

## Resources

**Intel Documentation:**
- [Intel® 64 and IA-32 Architectures Software Developer's Manual Volume 3C](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html) - VMX Chapters 23-33

**Specifications:**
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)

**Learning Resources:**
- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel VT-x Overview](https://www.intel.com/content/www/us/en/virtualization/virtualization-technology/intel-virtualization-technology.html)

**Community:**
- [OSDev Forums](https://forum.osdev.org/)
- [r/osdev](https://www.reddit.com/r/osdev/)

---

## License

Educational project - modify and use as you wish.

## Contributing

This is a learning project. Feel free to fork and experiment!

---

**Happy Hypervisor Hacking! 🚀**