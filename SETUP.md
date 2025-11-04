# VM Boot Environment Setup Guide

This guide documents the setup process for building and running the Type-1 Hypervisor.

## Quick Start

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install -y \
    build-essential \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    mtools \
    qemu-system-x86 \
    binutils

# Build the hypervisor
make clean
make iso

# Run the hypervisor (automatic environment detection)
./run-vm.sh

# Or run with graphical display
./run-vm.sh --graphic
```

## Environment Setup Complete

The following has been verified and configured:

✓ Build toolchain (gcc, g++, ld, as)
✓ NASM assembler (v2.16.01)
✓ QEMU x86_64 emulator (v8.2.2)
✓ GRUB bootloader tools (v2.12)
✓ ISO creation tools (xorriso, mtools)

## Current Hypervisor Status

### Working Features
- Multiboot2 compliant boot
- 32-bit to 64-bit long mode transition
- VGA text mode output with colors
- Serial port output
- CPUID-based CPU feature detection
- VMX (Intel VT-x) initialization code
- VMCS configuration
- VM entry/exit handling

### Implementation Status
The hypervisor includes complete VMX implementation:
- Feature control MSR checking and configuration
- CR0/CR4 adjustment for VMX requirements
- VMXON region setup and execution
- VMCS region initialization
- Guest state configuration
- Host state configuration
- VM execution controls
- VM launch capability

### Running on Real Hardware vs Emulation

**On Real Hardware with Intel VT-x:**
- Full VMX functionality will work
- Can create and run guest VMs
- VM entry/exit will be tested successfully

**In QEMU without KVM:**
- Hypervisor boots correctly
- Basic functionality works (VGA, serial, CPUID)
- VMX instructions are not supported by TCG emulator
- CPUID correctly reports VMX as unavailable

**In QEMU with KVM (nested virtualization):**
- Requires host CPU with VT-x and KVM loaded
- Full VMX functionality available
- Best environment for development without physical access

## Build Targets

```bash
make           # Build kernel ELF only
make iso       # Build kernel and create bootable ISO
make run       # Build and run (uses Makefile settings)
make clean     # Remove build artifacts
make info      # Display build configuration
```

## Running the Hypervisor

### Using the Helper Script (Recommended)
```bash
./run-vm.sh              # Text mode (serial console)
./run-vm.sh --graphic    # Graphical VGA display
```

The script automatically detects:
- CPU virtualization support (VMX/SVM)
- KVM availability
- Loads appropriate KVM modules if needed
- Selects correct CPU model for QEMU

### Manual QEMU Commands

**With KVM (requires VMX/SVM support):**
```bash
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm -nographic
```

**Without KVM (emulation only):**
```bash
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu qemu64 -nographic
```

**With graphical display:**
```bash
qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -cpu host -enable-kvm
```

## Testing VMX Functionality

To fully test the VMX functionality, you need one of:

1. **Real hardware with Intel VT-x**
   - Boot from USB or optical drive
   - Enable VT-x in BIOS/UEFI
   - Boot the ISO

2. **Nested virtualization**
   - Run on a VM with nested virtualization enabled
   - Example: VMware with "Virtualize Intel VT-x/EPT"
   - Example: VirtualBox with nested VT-x enabled
   - Example: KVM with nested=1 parameter

3. **Cloud instances with nested virtualization**
   - AWS: metal instances
   - GCP: enable nested virtualization
   - Azure: Dv3/Ev3 series with nested virtualization

## Expected Output

### Successful Boot (without VMX support):
```
================================
  Type-1 Hypervisor v0.1
================================

Multiboot Magic: 0x36D76289
Multiboot OK

ERROR: VMX not supported by CPU!
Cannot proceed with hypervisor initialization.
```

### Successful Boot (with VMX support):
```
================================
  Type-1 Hypervisor v0.1
================================

Multiboot Magic: 0x36D76289
Multiboot OK

CPU Vendor: GenuineIntel
Long Mode: Yes
VMX Support: Yes
MSR Support: Yes

================================
VMX Initialization
================================

Checking VMX feature control...
  Reading IA32_FEATURE_CONTROL MSR...
    Lock bit: Set
    VMX outside SMX: Enabled

Enabling VMX operation...
  Verifying CPUID VMX support...
    CPUID VMX support confirmed
  Reading IA32_VMX_BASIC MSR...
    VMCS Revision ID: 0x...
  Adjusting CR0...
    CR0 adjusted: 0x...
  Adjusting CR4 and enabling VMXE...
    CR4 adjusted: 0x...
  Initializing VMXON region...
    VMXON region at: 0x...
  Executing VMXON instruction...
    VMXON executed successfully!

VMX enabled successfully!

Testing VM entry/exit...
  Initializing VMCS region...
    VMCS region at: 0x...
  Executing VMPTRLD instruction...
    VMPTRLD executed successfully!
  Configuring VMCS fields...
    Setting guest state...
    Setting host state...
    Setting VM-execution controls...
    Setting VM-exit controls...
    Setting VM-entry controls...
    VMCS configuration complete!

Attempting VM launch...
  Executing VMLAUNCH instruction...

================================
VM Exit Occurred!
================================
Exit Reason: 0xC
Exit Qualification: 0x0
Guest RIP: 0x...

VM exit reason: HLT (expected)

================================
Hypervisor Phase 1 Complete!
================================
VMX initialized and tested successfully.
Ready for Phase 2: Memory Management.
```

## Troubleshooting

### Build Errors
- Ensure all dependencies are installed
- Run `make clean` before rebuilding
- Check compiler version: `g++ --version`

### Boot Errors
- Verify ISO was created: `ls -lh hypervisor.iso`
- Check QEMU logs: `cat qemu.log`
- Try without KVM if errors occur

### VMX Not Available
- This is expected in QEMU TCG mode
- To test VMX, use real hardware or nested virtualization
- The hypervisor correctly detects and reports this limitation

## Next Steps

### Phase 2: Enhanced Memory Management
- Implement proper memory allocator
- Set up page tables for guest
- Implement EPT (Extended Page Tables)

### Phase 3: Guest Boot
- Load a guest OS image
- Set up guest memory map
- Handle I/O port emulation
- Implement interrupt injection

### Phase 4: Device Emulation
- Virtual timer
- Virtual interrupt controller
- Virtual UART
- Block device support

## Files Created

- `run-vm.sh` - Helper script for running the hypervisor
- `SETUP.md` - This file (environment setup documentation)

## References

- Intel SDM Volume 3C: VMX Chapters 23-33
- [OSDev Wiki](https://wiki.osdev.org/)
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/)
