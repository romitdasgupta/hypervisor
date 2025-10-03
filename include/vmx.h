#ifndef VMX_H
#define VMX_H

#include "types.h"

// Intel VMX (Virtual Machine Extensions) support
// This is a placeholder for Phase 2

// MSR addresses for VMX
constexpr uint32_t IA32_FEATURE_CONTROL = 0x3A;
constexpr uint32_t IA32_VMX_BASIC = 0x480;
constexpr uint32_t IA32_VMX_CR0_FIXED0 = 0x486;
constexpr uint32_t IA32_VMX_CR0_FIXED1 = 0x487;
constexpr uint32_t IA32_VMX_CR4_FIXED0 = 0x488;
constexpr uint32_t IA32_VMX_CR4_FIXED1 = 0x489;

// Feature Control MSR bits
constexpr uint64_t FEATURE_CONTROL_LOCKED = (1ULL << 0);
constexpr uint64_t FEATURE_CONTROL_VMX_ENABLE = (1ULL << 2);

class VMX {
public:
    // Phase 2: We'll implement these
    static bool is_enabled();
    static bool enable();
    static bool check_feature_control();
    
private:
    static uint64_t read_msr(uint32_t msr);
    static void write_msr(uint32_t msr, uint64_t value);
};

#endif // VMX_H
