#include "types.h"
#include "vga.h"
#include "serial.h"
#include "cpuid.h"

// C++ runtime support - minimal implementation
extern "C" {
    // Required for pure virtual functions
    void __cxa_pure_virtual() {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("Pure virtual function called!\n");
        while(1) {
            asm volatile("hlt");
        }
    }

    // Required for global constructors
    void* __dso_handle = nullptr;

    int __cxa_atexit(void (*)(void*), void*, void*) {
        return 0;
    }
}

// Placement new operator
void* operator new(size_t, void* ptr) {
    return ptr;
}

void* operator new[](size_t, void* ptr) {
    return ptr;
}

// Entry point called from boot.S
extern "C" void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info) {
    // VGA is already initialized via global constructor
    vga.clear();
    
    // Print banner to both VGA and serial
    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("================================\n");
    vga.puts("  Type-1 Hypervisor v0.1\n");
    vga.puts("================================\n\n");

    // serial.puts("================================\n");
    // serial.puts("  Type-1 Hypervisor v0.1\n");
    // serial.puts("================================\n\n");

    vga.set_color(VGA::WHITE, VGA::BLACK);

    // Verify multiboot magic
    vga.puts("Multiboot Magic: ");
    //vga.put_hex(multiboot_magic);
    vga.putchar('\n');

    // serial.puts("Multiboot Magic: ");
    // serial.put_hex(multiboot_magic);
    // serial.putchar('\n');

    // if (multiboot_magic != 0x36d76289) {
    //     vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
    //     vga.puts("ERROR: Invalid multiboot magic!\n");
    //     serial.puts("ERROR: Invalid multiboot magic!\n");
    //     while(1) asm volatile("hlt");
    // }

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("Multiboot OK\n\n");
    // serial.puts("Multiboot OK\n\n");
    
    // Print CPU information
    vga.set_color(VGA::WHITE, VGA::BLACK);
    CPUID::print_cpu_info();

    // Check for VMX
    vga.putchar('\n');
    // serial.putchar('\n');
    if (!CPUID::has_vmx()) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("ERROR: VMX not supported by CPU!\n");
        vga.puts("Cannot proceed with hypervisor initialization.\n");
        // serial.puts("ERROR: VMX not supported by CPU!\n");
        // serial.puts("Cannot proceed with hypervisor initialization.\n");
        while(1) asm volatile("hlt");
    }

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("\nHypervisor initialization complete!\n");
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("Ready for VMX setup in next phase.\n");

    //serial.puts("\nHypervisor initialization complete!\n");
    //serial.puts("Ready for VMX setup in next phase.\n");

    // Halt - we'll add more functionality in next steps
    while(1) {
        asm volatile("hlt");
    }
}
