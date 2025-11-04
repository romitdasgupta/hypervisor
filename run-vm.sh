#!/bin/bash
# Helper script to run the hypervisor with appropriate settings

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}================================${NC}"
echo -e "${CYAN}  Hypervisor Boot Script${NC}"
echo -e "${CYAN}================================${NC}"
echo

# Check if ISO exists
if [ ! -f "hypervisor.iso" ]; then
    echo -e "${YELLOW}ISO not found. Building...${NC}"
    make iso
    echo
fi

# Check for hardware virtualization support
VMX_SUPPORT=$(egrep -c '(vmx|svm)' /proc/cpuinfo || true)

echo -e "${CYAN}System Information:${NC}"
if [ "$VMX_SUPPORT" -gt "0" ]; then
    echo -e "  CPU Virtualization: ${GREEN}Supported${NC} (VMX/SVM)"

    # Check if KVM is available
    if [ -e "/dev/kvm" ]; then
        echo -e "  KVM Device: ${GREEN}Available${NC}"
        USE_KVM=true
    else
        echo -e "  KVM Device: ${YELLOW}Not available${NC} (loading modules...)"
        sudo modprobe kvm_intel 2>/dev/null || sudo modprobe kvm_amd 2>/dev/null || true
        if [ -e "/dev/kvm" ]; then
            echo -e "  KVM Device: ${GREEN}Loaded${NC}"
            USE_KVM=true
        else
            echo -e "  KVM Device: ${RED}Failed to load${NC}"
            USE_KVM=false
        fi
    fi
else
    echo -e "  CPU Virtualization: ${YELLOW}Not supported${NC}"
    echo -e "  Note: Will run in emulation mode (slower, VMX features unavailable)"
    USE_KVM=false
fi

echo

# Build QEMU command
QEMU_CMD="qemu-system-x86_64 -cdrom hypervisor.iso -m 512M -boot d"

if [ "$USE_KVM" = true ]; then
    echo -e "${GREEN}Starting hypervisor with KVM acceleration...${NC}"
    QEMU_CMD="$QEMU_CMD -cpu host -enable-kvm"
else
    echo -e "${YELLOW}Starting hypervisor in emulation mode...${NC}"
    QEMU_CMD="$QEMU_CMD -cpu qemu64,+vmx"
    echo -e "${RED}Warning: TCG may not fully support VMX instructions${NC}"
fi

# Add display options
if [ "$1" == "--graphic" ]; then
    echo -e "Display: ${CYAN}Graphical${NC}"
    echo
else
    echo -e "Display: ${CYAN}Serial/Text${NC}"
    echo -e "(Use --graphic for GUI mode)"
    echo
    QEMU_CMD="$QEMU_CMD -nographic -serial mon:stdio"
fi

echo -e "${CYAN}Command:${NC} $QEMU_CMD"
echo -e "${CYAN}================================${NC}"
echo

# Add debug logging
QEMU_CMD="$QEMU_CMD -D qemu.log"

# Run QEMU
eval $QEMU_CMD

echo
echo -e "${GREEN}Hypervisor exited.${NC}"
echo -e "Logs saved to: qemu.log"
