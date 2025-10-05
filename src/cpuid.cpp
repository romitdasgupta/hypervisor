#include "cpuid.h"
#include "vga.h"
#include "serial.h"

CPUIDResult CPUID::query(uint32_t leaf, uint32_t subleaf) {
    CPUIDResult result;
    asm volatile("cpuid"
                 : "=a"(result.eax), "=b"(result.ebx),
                   "=c"(result.ecx), "=d"(result.edx)
                 : "a"(leaf), "c"(subleaf));
    return result;
}

bool CPUID::has_vmx() {
    // VMX is indicated by ECX bit 5 of CPUID.1
    CPUIDResult result = query(1);
    return (result.ecx & (1 << 5)) != 0;
}

bool CPUID::has_long_mode() {
    // Check if extended CPUID is available
    CPUIDResult result = query(0x80000000);
    if (result.eax < 0x80000001) {
        return false;
    }
    
    // Long mode is EDX bit 29 of CPUID.0x80000001
    result = query(0x80000001);
    return (result.edx & (1 << 29)) != 0;
}

bool CPUID::has_msr() {
    // MSR is indicated by EDX bit 5 of CPUID.1
    CPUIDResult result = query(1);
    return (result.edx & (1 << 5)) != 0;
}

void CPUID::get_vendor_string(char* str) {
    CPUIDResult result = query(0);
    
    // EBX, EDX, ECX contain vendor string
    *reinterpret_cast<uint32_t*>(str + 0) = result.ebx;
    *reinterpret_cast<uint32_t*>(str + 4) = result.edx;
    *reinterpret_cast<uint32_t*>(str + 8) = result.ecx;
    str[12] = '\0';
}

void CPUID::print_cpu_info() {
    char vendor[13];
    get_vendor_string(vendor);
    
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("CPU Vendor: ");
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts(vendor);
    vga.putchar('\n');
    
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("Long Mode: ");
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts(has_long_mode() ? "Yes\n" : "No\n");
    
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("VMX Support: ");
    vga.set_color(has_vmx() ? VGA::LIGHT_GREEN : VGA::LIGHT_RED, VGA::BLACK);
    vga.puts(has_vmx() ? "Yes\n" : "No\n");
    
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("MSR Support: ");
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts(has_msr() ? "Yes\n" : "No\n");
}
