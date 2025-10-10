#include "types.h"
#include "vmx.h"
#include "vga.h"
#include "serial.h"
#include "cpuid.h"

// ============================================================================
// CPU Primitive Functions (Static Helper Functions)
// ============================================================================

static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((uint64_t)high << 32) | low;
}

static inline void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile(
        "wrmsr"
        :
        : "c"(msr), "a"(low), "d"(high)
    );
}

static inline uint64_t read_cr0() {
    uint64_t value;
    asm volatile(
        "mov %%cr0, %0"
        : "=r"(value)
    );
    return value;
}

static inline void write_cr0(uint64_t value) {
    asm volatile(
        "mov %0, %%cr0"
        :
        : "r"(value)
    );
}

static inline uint64_t read_cr3() {
    uint64_t value;
    asm volatile(
        "mov %%cr3, %0"
        : "=r"(value)
    );
    return value;
}

static inline uint64_t read_cr4() {
    uint64_t value;
    asm volatile(
        "mov %%cr4, %0"
        : "=r"(value)
    );
    return value;
}

static inline void write_cr4(uint64_t value) {
    asm volatile(
        "mov %0, %%cr4"
        :
        : "r"(value)
    );
}

static inline uint64_t read_rsp() {
    uint64_t value;
    asm volatile(
        "mov %%rsp, %0"
        : "=r"(value)
    );
    return value;
}

static inline uint16_t read_tr() {
    uint16_t tr;
    asm volatile(
        "str %0"
        : "=r"(tr)
    );
    return tr;
}

struct DescriptorTableRegister {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static inline void read_gdtr(DescriptorTableRegister* gdtr) {
    asm volatile(
        "sgdt %0"
        : "=m"(*gdtr)
    );
}

static inline void read_idtr(DescriptorTableRegister* idtr) {
    asm volatile(
        "sidt %0"
        : "=m"(*idtr)
    );
}

static inline uint64_t read_fs_base() {
    uint32_t low, high;
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(0xC0000100)  // IA32_FS_BASE MSR
    );
    return ((uint64_t)high << 32) | low;
}

static inline uint64_t read_gs_base() {
    uint32_t low, high;
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(0xC0000101)  // IA32_GS_BASE MSR
    );
    return ((uint64_t)high << 32) | low;
}

// ============================================================================
// Static Memory Regions
// ============================================================================

// VMX regions must be page-aligned and in dedicated section to avoid BSS clearing
alignas(4096) __attribute__((section(".vmx_regions"))) static uint8_t vmxon_region[4096];
alignas(4096) __attribute__((section(".vmx_regions"))) static uint8_t vmcs_region[4096];

// Guest stack can remain in normal sections
alignas(4096) static uint8_t guest_stack[8192];

static bool vmx_enabled = false;
static uint32_t vmcs_revision_id = 0;

// ============================================================================
// Forward Declarations
// ============================================================================

static bool vmcs_init();
static bool setup_vmcs();
static bool vmx_launch();
extern "C" void vm_exit_handler();
extern "C" void guest_entry_point();

// ============================================================================
// VMX Class Method Implementations
// ============================================================================

uint64_t VMX::read_msr(uint32_t msr) {
    return ::read_msr(msr);
}

void VMX::write_msr(uint32_t msr, uint64_t value) {
    ::write_msr(msr, value);
}

void VMX::check_mtrr_config() {
    // Check if MTRRs are supported
    uint64_t mtrr_cap = read_msr(0xFE);
    vga.puts("MTRR variable ranges: ");
    vga.put_hex(mtrr_cap & 0xFF);
    vga.puts("\n");
    serial.puts("MTRR variable ranges: ");
    serial.put_hex(mtrr_cap & 0xFF);
    serial.puts("\n");

    // Check default memory type
    uint64_t def_type = read_msr(0x2FF);
    bool mtrr_enabled = def_type & (1 << 11);
    uint8_t default_type = def_type & 0xFF;

    vga.puts("MTRR enabled: ");
    vga.puts(mtrr_enabled ? "Yes\n" : "No\n");
    vga.puts("Default type: 0x");
    vga.put_hex(default_type);
    vga.puts(default_type == 6 ? " (WB)\n" : "\n");
    serial.puts("MTRR enabled: ");
    serial.puts(mtrr_enabled ? "Yes\n" : "No\n");
    serial.puts("Default type: 0x");
    serial.put_hex(default_type);
    serial.puts(default_type == 6 ? " (WB)\n" : "\n");
}

bool VMX::check_feature_control() {
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Reading IA32_FEATURE_CONTROL MSR...\n");
    serial.puts("  Reading IA32_FEATURE_CONTROL MSR...\n");

    uint64_t feature_control = read_msr(IA32_FEATURE_CONTROL);

    // Check bit 0 (locked)
    bool locked = feature_control & FEATURE_CONTROL_LOCKED;
    vga.puts("    Lock bit: ");
    serial.puts("    Lock bit: ");
    if (locked) {
        vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
        vga.puts("Set\n");
        serial.puts("Set\n");
    } else {
        vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
        vga.puts("Not Set (unlocked)\n");
        serial.puts("Not Set (unlocked)\n");
    }

    // Check bit 2 (VMX outside SMX)
    bool vmx_outside_smx = feature_control & FEATURE_CONTROL_VMX_ENABLE;
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("    VMX outside SMX: ");
    serial.puts("    VMX outside SMX: ");
    if (vmx_outside_smx) {
        vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
        vga.puts("Enabled\n");
        serial.puts("Enabled\n");
    } else {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("Disabled\n");
        serial.puts("Disabled\n");
    }

    // If unlocked and VMX not enabled, attempt to configure it
    if (!locked && !vmx_outside_smx) {
        vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
        vga.puts("  Attempting to configure IA32_FEATURE_CONTROL...\n");
        serial.puts("  Attempting to configure IA32_FEATURE_CONTROL...\n");

        // Verify CPUID MSR support (running at CPL0 is already ensured by kernel context)
        if (!CPUID::has_msr()) {
            vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
            vga.puts("    ERROR: CPU does not support MSRs!\n");
            serial.puts("    ERROR: CPU does not support MSRs!\n");
            return false;
        }

        // Write IA32_FEATURE_CONTROL with VMX enabled and locked
        uint64_t new_value = FEATURE_CONTROL_VMX_ENABLE | FEATURE_CONTROL_LOCKED;
        write_msr(IA32_FEATURE_CONTROL, new_value);

        // Re-read to confirm the write succeeded
        feature_control = read_msr(IA32_FEATURE_CONTROL);
        locked = feature_control & FEATURE_CONTROL_LOCKED;
        vmx_outside_smx = feature_control & FEATURE_CONTROL_VMX_ENABLE;

        if (locked && vmx_outside_smx) {
            vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
            vga.puts("    Successfully configured IA32_FEATURE_CONTROL!\n");
            serial.puts("    Successfully configured IA32_FEATURE_CONTROL!\n");
            vga.puts("    Lock bit: Set, VMX: Enabled\n");
            serial.puts("    Lock bit: Set, VMX: Enabled\n");
        } else {
            vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
            vga.puts("    WARNING: Failed to configure IA32_FEATURE_CONTROL\n");
            serial.puts("    WARNING: Failed to configure IA32_FEATURE_CONTROL\n");
            vga.puts("    (MSR write may have been ignored by firmware)\n");
            serial.puts("    (MSR write may have been ignored by firmware)\n");
            return false;
        }
    }

    vga.set_color(VGA::WHITE, VGA::BLACK);
    return locked && vmx_outside_smx;
}

bool VMX::is_enabled() {
    return vmx_enabled;
}

bool VMX::enable() {
    // Step 1: Verify CPUID VMX support
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Verifying CPUID VMX support...\n");
    serial.puts("  Verifying CPUID VMX support...\n");

    if (!CPUID::has_vmx()) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("    ERROR: CPUID VMX support not found!\n");
        serial.puts("    ERROR: CPUID VMX support not found!\n");
        return false;
    }

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    CPUID VMX support confirmed\n");
    serial.puts("    CPUID VMX support confirmed\n");

    // Step 2: Check MTRR configuration (diagnostic)
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Checking MTRR configuration...\n");
    serial.puts("  Checking MTRR configuration...\n");
    check_mtrr_config();

    // Step 3: Read IA32_VMX_BASIC to get VMCS revision ID
    // (Feature control already checked by caller)
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Reading IA32_VMX_BASIC MSR...\n");
    serial.puts("  Reading IA32_VMX_BASIC MSR...\n");

    uint64_t vmx_basic = read_msr(IA32_VMX_BASIC);
    vmcs_revision_id = vmx_basic & 0x7FFFFFFF;  // Bits 30:0

    vga.puts("    VMCS Revision ID: 0x");
    serial.puts("    VMCS Revision ID: 0x");
    vga.put_hex(vmcs_revision_id);
    serial.put_hex(vmcs_revision_id);
    vga.puts("\n");
    serial.puts("\n");

    // Step 4: Adjust CR0
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Adjusting CR0...\n");
    serial.puts("  Adjusting CR0...\n");

    uint64_t cr0 = read_cr0();
    uint64_t cr0_fixed0 = read_msr(IA32_VMX_CR0_FIXED0);
    uint64_t cr0_fixed1 = read_msr(IA32_VMX_CR0_FIXED1);
    cr0 = (cr0 | cr0_fixed0) & cr0_fixed1;
    write_cr0(cr0);

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    CR0 adjusted: 0x");
    serial.puts("    CR0 adjusted: 0x");
    vga.put_hex(cr0);
    serial.put_hex64(cr0);
    vga.puts("\n");
    serial.puts("\n");

    // Step 5: Adjust CR4 and set VMXE bit
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Adjusting CR4 and enabling VMXE...\n");
    serial.puts("  Adjusting CR4 and enabling VMXE...\n");

    uint64_t cr4 = read_cr4();
    uint64_t cr4_fixed0 = read_msr(IA32_VMX_CR4_FIXED0);
    uint64_t cr4_fixed1 = read_msr(IA32_VMX_CR4_FIXED1);
    cr4 = (cr4 | cr4_fixed0) & cr4_fixed1;
    cr4 |= (1ULL << 13);  // Set CR4.VMXE (bit 13)
    write_cr4(cr4);

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    CR4 adjusted: 0x");
    serial.puts("    CR4 adjusted: 0x");
    vga.put_hex(cr4);
    serial.put_hex64(cr4);
    vga.puts("\n");
    serial.puts("\n");

    // Step 6: Initialize VMXON region
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Initializing VMXON region...\n");
    serial.puts("  Initializing VMXON region...\n");

    // Clear VMXON region
    for (int i = 0; i < 4096; i++) {
        vmxon_region[i] = 0;
    }

    // Write revision ID to first 4 bytes, ensure bit 31 is clear
    uint32_t* vmxon_rev = (uint32_t*)vmxon_region;
    *vmxon_rev = vmcs_revision_id & 0x7FFFFFFF;

    vga.puts("    VMXON region at: 0x");
    serial.puts("    VMXON region at: 0x");
    vga.put_hex((uint64_t)vmxon_region);
    serial.put_hex64((uint64_t)vmxon_region);
    vga.puts("\n");
    serial.puts("\n");

    // Step 7: Execute VMXON instruction
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Executing VMXON instruction...\n");
    serial.puts("  Executing VMXON instruction...\n");

    uint64_t vmxon_physical_addr = (uint64_t)vmxon_region;  // Identity mapping
    uint8_t error = 0;

    asm volatile(
        "vmxon (%1)\n\t"
        "setna %0"  // Set if CF=1 or ZF=1 (error condition)
        : "=r"(error)
        : "r"(&vmxon_physical_addr)
        : "cc", "memory"
    );

    if (error) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("    ERROR: VMXON instruction failed!\n");
        serial.puts("    ERROR: VMXON instruction failed!\n");
        return false;
    }

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    VMXON executed successfully!\n");
    serial.puts("    VMXON executed successfully!\n");

    vmx_enabled = true;
    return true;
}

// ============================================================================
// VMCS Management Functions
// ============================================================================

static bool vmcs_write(uint64_t field, uint64_t value) {
    uint8_t error = 0;
    asm volatile(
        "vmwrite %1, %2\n\t"
        "setna %0"
        : "=r"(error)
        : "r"(value), "r"(field)
        : "cc"
    );
    return (error == 0);
}

static uint64_t vmcs_read(uint64_t field) {
    uint64_t value = 0;
    asm volatile(
        "vmread %1, %0"
        : "=r"(value)
        : "r"(field)
        : "cc"
    );
    return value;
}

static bool vmcs_init() {
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Initializing VMCS region...\n");
    serial.puts("  Initializing VMCS region...\n");

    // Clear VMCS region
    for (int i = 0; i < 4096; i++) {
        vmcs_region[i] = 0;
    }

    // Write revision ID to first 4 bytes, ensure bit 31 is clear
    uint32_t* vmcs_rev = (uint32_t*)vmcs_region;
    *vmcs_rev = vmcs_revision_id & 0x7FFFFFFF;

    vga.puts("    VMCS region at: 0x");
    serial.puts("    VMCS region at: 0x");
    vga.put_hex((uint64_t)vmcs_region);
    serial.put_hex64((uint64_t)vmcs_region);
    vga.puts("\n");
    serial.puts("\n");

    // Execute VMPTRLD instruction
    vga.puts("  Executing VMPTRLD instruction...\n");
    serial.puts("  Executing VMPTRLD instruction...\n");

    uint64_t vmcs_physical_addr = (uint64_t)vmcs_region;  // Identity mapping
    uint8_t error = 0;

    asm volatile(
        "vmptrld (%1)\n\t"
        "setna %0"
        : "=r"(error)
        : "r"(&vmcs_physical_addr)
        : "cc", "memory"
    );

    if (error) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("    ERROR: VMPTRLD instruction failed!\n");
        serial.puts("    ERROR: VMPTRLD instruction failed!\n");
        return false;
    }

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    VMPTRLD executed successfully!\n");
    serial.puts("    VMPTRLD executed successfully!\n");

    return true;
}

static bool setup_vmcs() {
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Configuring VMCS fields...\n");
    serial.puts("  Configuring VMCS fields...\n");

    // Guest state - 16-bit real mode style
    vga.puts("    Setting guest state...\n");
    serial.puts("    Setting guest state...\n");

    // Guest segment selectors
    if (!vmcs_write(GUEST_CS_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_SS_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_DS_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_ES_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_FS_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_GS_SELECTOR, 0)) return false;

    // Guest segment bases
    if (!vmcs_write(GUEST_CS_BASE, 0)) return false;
    if (!vmcs_write(GUEST_SS_BASE, 0)) return false;
    if (!vmcs_write(GUEST_DS_BASE, 0)) return false;
    if (!vmcs_write(GUEST_ES_BASE, 0)) return false;
    if (!vmcs_write(GUEST_FS_BASE, 0)) return false;
    if (!vmcs_write(GUEST_GS_BASE, 0)) return false;

    // Guest segment limits (0xFFFF for 16-bit segments)
    if (!vmcs_write(GUEST_CS_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_SS_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_DS_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_ES_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_FS_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_GS_LIMIT, 0xFFFF)) return false;

    // Guest segment access rights
    // 0x93 = Present, DPL=0, Data segment, writable
    // 0x9B = Present, DPL=0, Code segment, readable
    if (!vmcs_write(GUEST_CS_ACCESS_RIGHTS, 0x9B)) return false;
    if (!vmcs_write(GUEST_SS_ACCESS_RIGHTS, 0x93)) return false;
    if (!vmcs_write(GUEST_DS_ACCESS_RIGHTS, 0x93)) return false;
    if (!vmcs_write(GUEST_ES_ACCESS_RIGHTS, 0x93)) return false;
    if (!vmcs_write(GUEST_FS_ACCESS_RIGHTS, 0x93)) return false;
    if (!vmcs_write(GUEST_GS_ACCESS_RIGHTS, 0x93)) return false;

    // Guest LDTR (marked as unusable)
    if (!vmcs_write(GUEST_LDTR_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_LDTR_BASE, 0)) return false;
    if (!vmcs_write(GUEST_LDTR_LIMIT, 0)) return false;
    if (!vmcs_write(GUEST_LDTR_ACCESS_RIGHTS, 0x10000)) return false;  // Bit 16 = unusable

    // Guest TR (must be valid for VM entry)
    // Set up a minimal valid TR descriptor
    if (!vmcs_write(GUEST_TR_SELECTOR, 0)) return false;
    if (!vmcs_write(GUEST_TR_BASE, 0)) return false;
    if (!vmcs_write(GUEST_TR_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_TR_ACCESS_RIGHTS, 0x8B)) return false;  // Present, DPL=0, 32-bit TSS (busy)

    // Guest GDTR and IDTR
    if (!vmcs_write(GUEST_GDTR_BASE, 0)) return false;
    if (!vmcs_write(GUEST_GDTR_LIMIT, 0xFFFF)) return false;
    if (!vmcs_write(GUEST_IDTR_BASE, 0)) return false;
    if (!vmcs_write(GUEST_IDTR_LIMIT, 0xFFFF)) return false;

    // Guest RSP (top of guest stack)
    uint64_t guest_rsp = (uint64_t)guest_stack + 8192;
    if (!vmcs_write(GUEST_RSP, guest_rsp)) return false;

    // Guest RIP (entry point)
    uint64_t guest_rip = (uint64_t)guest_entry_point;
    if (!vmcs_write(GUEST_RIP, guest_rip)) return false;

    // Guest RFLAGS (bit 1 must be 1)
    if (!vmcs_write(GUEST_RFLAGS, 0x2)) return false;

    // Guest CR0 - Must respect VMX fixed bits
    // Start with desired bits: ET (bit 4) + NE (bit 5) for real mode
    // PE=0 (real mode), PG=0 (no paging), CD=0, NW=0 (caches enabled)
    // Apply VMX fixed bits to ensure compliance
    uint64_t guest_cr0_fixed0 = read_msr(IA32_VMX_CR0_FIXED0);
    uint64_t guest_cr0_fixed1 = read_msr(IA32_VMX_CR0_FIXED1);
    uint64_t guest_cr0 = 0x00000030;  // ET + NE
    guest_cr0 = (guest_cr0 | guest_cr0_fixed0) & guest_cr0_fixed1;
    if (!vmcs_write(GUEST_CR0, guest_cr0)) return false;

    // Guest CR3
    if (!vmcs_write(GUEST_CR3, 0)) return false;

    // Guest CR4 - Must respect VMX fixed bits (VMXE must NOT be set in guest)
    // See Intel SDM Vol 3C "Checks on Guest Control Registers, Debug Registers, and MSRs"
    uint64_t guest_cr4_fixed0 = read_msr(IA32_VMX_CR4_FIXED0);
    uint64_t guest_cr4_fixed1 = read_msr(IA32_VMX_CR4_FIXED1);
    uint64_t guest_cr4 = 0;  // Start with no features enabled
    guest_cr4 = (guest_cr4 | guest_cr4_fixed0) & guest_cr4_fixed1;
    guest_cr4 &= ~(1ULL << 13);  // Ensure VMXE (bit 13) is clear for guest
    if (!vmcs_write(GUEST_CR4, guest_cr4)) return false;

    // VMCS link pointer - set to ~0ULL for no shadowing
    if (!vmcs_write(VMCS_LINK_POINTER, ~0ULL)) return false;

    // Host state
    vga.puts("    Setting host state...\n");
    serial.puts("    Setting host state...\n");

    // Host segment selectors (from GDT in boot.S)
    if (!vmcs_write(HOST_CS_SELECTOR, 0x08)) return false;
    if (!vmcs_write(HOST_SS_SELECTOR, 0x10)) return false;
    if (!vmcs_write(HOST_DS_SELECTOR, 0x10)) return false;
    if (!vmcs_write(HOST_ES_SELECTOR, 0x10)) return false;
    if (!vmcs_write(HOST_FS_SELECTOR, 0x10)) return false;
    if (!vmcs_write(HOST_GS_SELECTOR, 0x10)) return false;
    
    // Host TR selector (use current TR)
    uint16_t host_tr = read_tr();
    if (!vmcs_write(HOST_TR_SELECTOR, host_tr)) return false;

    // Host RSP (current stack)
    uint64_t host_rsp = read_rsp();
    if (!vmcs_write(HOST_RSP, host_rsp)) return false;

    // Host RIP (VM exit handler)
    uint64_t host_rip = (uint64_t)vm_exit_handler;
    if (!vmcs_write(HOST_RIP, host_rip)) return false;

    // Host CR0, CR3, CR4 (current values)
    if (!vmcs_write(HOST_CR0, read_cr0())) return false;
    if (!vmcs_write(HOST_CR3, read_cr3())) return false;
    if (!vmcs_write(HOST_CR4, read_cr4())) return false;

    // Host FS/GS bases (read from MSRs)
    uint64_t host_fs_base = read_fs_base();
    uint64_t host_gs_base = read_gs_base();
    if (!vmcs_write(HOST_FS_BASE, host_fs_base)) return false;
    if (!vmcs_write(HOST_GS_BASE, host_gs_base)) return false;

    // Host GDTR and IDTR bases
    DescriptorTableRegister gdtr, idtr;
    read_gdtr(&gdtr);
    read_idtr(&idtr);
    if (!vmcs_write(HOST_GDTR_BASE, gdtr.base)) return false;
    if (!vmcs_write(HOST_IDTR_BASE, idtr.base)) return false;

    // Host TR base (computed from GDT)
    // The TR selector indexes into the GDT. Each descriptor is 16 bytes for 64-bit TSS.
    // Descriptor format: bytes 0-1: limit[15:0], bytes 2-3: base[15:0], byte 4: base[23:16],
    // byte 5: access byte, byte 6: limit[19:16]+flags, byte 7: base[31:24],
    // bytes 8-11: base[63:32] (for 64-bit system descriptors)
    uint16_t tr_index = (host_tr & 0xFFF8) >> 3;  // Extract index (bits 15:3)
    uint64_t descriptor_addr = gdtr.base + (tr_index * 8);  // Each GDT entry is 8 bytes offset
    uint8_t* desc = (uint8_t*)descriptor_addr;
    
    // Extract 64-bit base from system descriptor (16 bytes total)
    uint64_t host_tr_base = 0;
    host_tr_base |= ((uint64_t)desc[2]);           // base[15:8]
    host_tr_base |= ((uint64_t)desc[3] << 8);      // base[23:16]
    host_tr_base |= ((uint64_t)desc[4] << 16);     // base[31:24]
    host_tr_base |= ((uint64_t)desc[7] << 24);     // base[39:32]
    // For 64-bit system descriptor, base[63:32] is in the next 8 bytes
    uint32_t* desc32 = (uint32_t*)(desc + 8);
    host_tr_base |= ((uint64_t)desc32[0] << 32);   // base[63:32]
    
    if (!vmcs_write(HOST_TR_BASE, host_tr_base)) return false;

    // VM-execution controls
    vga.puts("    Setting VM-execution controls...\n");
    serial.puts("    Setting VM-execution controls...\n");

    // PIN-based controls
    uint64_t pinbased_ctls = read_msr(IA32_VMX_PINBASED_CTLS);
    uint32_t pinbased_allowed0 = (uint32_t)pinbased_ctls;         // bits that can be 0; 0 means must be 1
    uint32_t pinbased_allowed1 = (uint32_t)(pinbased_ctls >> 32); // bits that can be 1; 0 means must be 0
    uint32_t pinbased_must_be_one = ~pinbased_allowed0;
    uint32_t pinbased_must_be_zero = ~pinbased_allowed1;
    uint32_t pinbased_desired = 0;  // minimal set for now
    uint32_t pinbased_controls = (pinbased_desired | pinbased_must_be_one) & ~pinbased_must_be_zero;
    if (!vmcs_write(PIN_BASED_VM_EXEC_CONTROL, pinbased_controls)) return false;

    // Processor-based controls
    uint64_t procbased_ctls = read_msr(IA32_VMX_PROCBASED_CTLS);
    uint32_t procbased_allowed0 = (uint32_t)procbased_ctls;
    uint32_t procbased_allowed1 = (uint32_t)(procbased_ctls >> 32);
    uint32_t procbased_must_be_one = ~procbased_allowed0;
    uint32_t procbased_must_be_zero = ~procbased_allowed1;
    // Bit 7: HLT exiting (so guest HLT triggers exit)
    // Bit 31: Activate secondary controls
    uint32_t procbased_desired = (1 << 7) | (1 << 31);
    uint32_t procbased_controls = (procbased_desired | procbased_must_be_one) & ~procbased_must_be_zero;
    if (!vmcs_write(PROC_BASED_VM_EXEC_CONTROL, procbased_controls)) return false;

    // Secondary processor-based controls (for unrestricted guest)
    uint64_t procbased2_ctls = read_msr(IA32_VMX_PROCBASED_CTLS2);
    uint32_t procbased2_allowed0 = (uint32_t)procbased2_ctls;
    uint32_t procbased2_allowed1 = (uint32_t)(procbased2_ctls >> 32);
    uint32_t procbased2_must_be_one = ~procbased2_allowed0;
    uint32_t procbased2_must_be_zero = ~procbased2_allowed1;
    // Bit 7: Unrestricted guest (allows real-mode guest with invalid state)
    uint32_t procbased2_desired = (1 << 7);
    uint32_t procbased2_controls = (procbased2_desired | procbased2_must_be_one) & ~procbased2_must_be_zero;
    if (!vmcs_write(SECONDARY_VM_EXEC_CONTROL, procbased2_controls)) return false;

    if (!vmcs_write(EXCEPTION_BITMAP, 0)) return false;

    // VM-exit controls
    vga.puts("    Setting VM-exit controls...\n");
    serial.puts("    Setting VM-exit controls...\n");

    uint64_t exit_ctls = read_msr(IA32_VMX_EXIT_CTLS);
    uint32_t exit_allowed0 = (uint32_t)exit_ctls;         // bits that can be 0; 0 means must be 1
    uint32_t exit_allowed1 = (uint32_t)(exit_ctls >> 32); // bits that can be 1; 0 means must be 0
    uint32_t exit_must_be_one = ~exit_allowed0;
    uint32_t exit_must_be_zero = ~exit_allowed1;
    uint32_t exit_desired = (1 << 9);  // Host address-space size (64-bit)
    uint32_t exit_controls = (exit_desired | exit_must_be_one) & ~exit_must_be_zero;
    if (!vmcs_write(VM_EXIT_CONTROLS, exit_controls)) return false;

    // VM-entry controls
    vga.puts("    Setting VM-entry controls...\n");
    serial.puts("    Setting VM-entry controls...\n");

    uint64_t entry_ctls = read_msr(IA32_VMX_ENTRY_CTLS);
    uint32_t entry_allowed0 = (uint32_t)entry_ctls;         // bits that can be 0; 0 means must be 1
    uint32_t entry_allowed1 = (uint32_t)(entry_ctls >> 32); // bits that can be 1; 0 means must be 0
    uint32_t entry_must_be_one = ~entry_allowed0;
    uint32_t entry_must_be_zero = ~entry_allowed1;
    uint32_t entry_desired = 0;  // IA-32e mode guest = 0 (don't set bit 9)
    uint32_t entry_controls = (entry_desired | entry_must_be_one) & ~entry_must_be_zero;
    if (!vmcs_write(VM_ENTRY_CONTROLS, entry_controls)) return false;

    vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
    vga.puts("    VMCS configuration complete!\n");
    serial.puts("    VMCS configuration complete!\n");

    return true;
}

// ============================================================================
// Guest Entry Point
// ============================================================================

extern "C" void guest_entry_point() {
    // Minimal guest code that triggers VM exit via HLT
    asm volatile("hlt");
    while (1) {
        asm volatile("hlt");
    }
}

// ============================================================================
// VM Exit Handler
// ============================================================================

extern "C" void vm_exit_handler() {
    // Read VM-exit information
    uint64_t exit_reason = vmcs_read(VM_EXIT_REASON);
    uint64_t exit_qualification = vmcs_read(EXIT_QUALIFICATION);
    uint64_t guest_rip = vmcs_read(GUEST_RIP);

    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("\n================================\n");
    vga.puts("VM Exit Occurred!\n");
    vga.puts("================================\n");
    serial.puts("\n================================\n");
    serial.puts("VM Exit Occurred!\n");
    serial.puts("================================\n");

    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("Exit Reason: 0x");
    vga.put_hex(exit_reason);
    vga.puts("\n");
    serial.puts("Exit Reason: 0x");
    serial.put_hex64(exit_reason);
    serial.puts("\n");

    vga.puts("Exit Qualification: 0x");
    vga.put_hex(exit_qualification);
    vga.puts("\n");
    serial.puts("Exit Qualification: 0x");
    serial.put_hex64(exit_qualification);
    serial.puts("\n");

    vga.puts("Guest RIP: 0x");
    vga.put_hex(guest_rip);
    vga.puts("\n");
    serial.puts("Guest RIP: 0x");
    serial.put_hex64(guest_rip);
    serial.puts("\n");

    // Check if exit reason is HLT (12)
    if ((exit_reason & 0xFFFF) == 12) {
        vga.set_color(VGA::LIGHT_GREEN, VGA::BLACK);
        vga.puts("\nVM exit reason: HLT (expected)\n");
        serial.puts("\nVM exit reason: HLT (expected)\n");
    } else {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("\nVM exit reason: Unexpected!\n");
        serial.puts("\nVM exit reason: Unexpected!\n");
    }

    // Halt
    while (1) {
        asm volatile("hlt");
    }
}

// ============================================================================
// VM Launch Function
// ============================================================================

static bool vmx_launch() {
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("  Executing VMLAUNCH instruction...\n");
    serial.puts("  Executing VMLAUNCH instruction...\n");

    uint8_t error = 0;
    asm volatile(
        "vmlaunch\n\t"
        "setna %0"
        : "=r"(error)
        :
        : "cc", "memory"
    );

    if (error) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("    ERROR: VMLAUNCH instruction failed!\n");
        serial.puts("    ERROR: VMLAUNCH instruction failed!\n");

        uint64_t vm_error = vmcs_read(VM_INSTRUCTION_ERROR);
        vga.puts("    VM-instruction error: 0x");
        vga.put_hex(vm_error);
        vga.puts("\n");
        serial.puts("    VM-instruction error: 0x");
        serial.put_hex64(vm_error);
        serial.puts("\n");

        return false;
    }

    // Should never reach here on success
    return true;
}

// ============================================================================
// Main VMX Test Function
// ============================================================================

bool VMX::test_vmx() {
    vga.set_color(VGA::LIGHT_CYAN, VGA::BLACK);
    vga.puts("\n================================\n");
    vga.puts("VMX Test: VMCS Init & VM Launch\n");
    vga.puts("================================\n");
    serial.puts("\n================================\n");
    serial.puts("VMX Test: VMCS Init & VM Launch\n");
    serial.puts("================================\n");

    // Initialize VMCS
    if (!vmcs_init()) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("ERROR: VMCS initialization failed!\n");
        serial.puts("ERROR: VMCS initialization failed!\n");
        return false;
    }

    // Setup VMCS fields
    if (!setup_vmcs()) {
        vga.set_color(VGA::LIGHT_RED, VGA::BLACK);
        vga.puts("ERROR: VMCS setup failed!\n");
        serial.puts("ERROR: VMCS setup failed!\n");
        return false;
    }

    // Attempt VM launch
    vga.set_color(VGA::WHITE, VGA::BLACK);
    vga.puts("\nAttempting VM launch...\n");
    serial.puts("\nAttempting VM launch...\n");

    if (!vmx_launch()) {
        return false;
    }

    // Should not reach here if VMLAUNCH succeeded
    return true;
}
