#ifndef CPUID_H
#define CPUID_H

#include "types.h"

struct CPUIDResult {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

class CPUID {
public:
    static CPUIDResult query(uint32_t leaf, uint32_t subleaf = 0);
    
    // Feature detection
    static bool has_vmx();        // Intel VT-x
    static bool has_long_mode();  // 64-bit support
    static bool has_msr();        // Model-Specific Registers
    
    static void print_cpu_info();
    
private:
    static void get_vendor_string(char* str);
};

#endif // CPUID_H
