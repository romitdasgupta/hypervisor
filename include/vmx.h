#ifndef VMX_H
#define VMX_H

#include "types.h"

// Intel VMX (Virtual Machine Extensions) implementation - Phase 1: Basic VMX initialization and VM entry/exit

// MSR addresses for VMX
constexpr uint32_t IA32_FEATURE_CONTROL = 0x3A;
constexpr uint32_t IA32_VMX_BASIC = 0x480;
constexpr uint32_t IA32_VMX_CR0_FIXED0 = 0x486;
constexpr uint32_t IA32_VMX_CR0_FIXED1 = 0x487;
constexpr uint32_t IA32_VMX_CR4_FIXED0 = 0x488;
constexpr uint32_t IA32_VMX_CR4_FIXED1 = 0x489;

// VM-execution control MSRs
constexpr uint32_t IA32_VMX_PINBASED_CTLS = 0x481;
constexpr uint32_t IA32_VMX_PROCBASED_CTLS = 0x482;
constexpr uint32_t IA32_VMX_PROCBASED_CTLS2 = 0x48B;
constexpr uint32_t IA32_VMX_EXIT_CTLS = 0x483;
constexpr uint32_t IA32_VMX_ENTRY_CTLS = 0x484;

// Feature Control MSR bits
constexpr uint64_t FEATURE_CONTROL_LOCKED = (1ULL << 0);
constexpr uint64_t FEATURE_CONTROL_VMX_ENABLE = (1ULL << 2);

// VMCS field encodings (Intel SDM Vol 3C Appendix B)

// Guest state fields - 16-bit selectors
constexpr uint64_t GUEST_ES_SELECTOR = 0x0800;
constexpr uint64_t GUEST_CS_SELECTOR = 0x0802;
constexpr uint64_t GUEST_SS_SELECTOR = 0x0804;
constexpr uint64_t GUEST_DS_SELECTOR = 0x0806;
constexpr uint64_t GUEST_FS_SELECTOR = 0x0808;
constexpr uint64_t GUEST_GS_SELECTOR = 0x080A;
constexpr uint64_t GUEST_LDTR_SELECTOR = 0x080C;
constexpr uint64_t GUEST_TR_SELECTOR = 0x080E;

// Guest state fields - 32-bit limits
constexpr uint64_t GUEST_ES_LIMIT = 0x4800;
constexpr uint64_t GUEST_CS_LIMIT = 0x4802;
constexpr uint64_t GUEST_SS_LIMIT = 0x4804;
constexpr uint64_t GUEST_DS_LIMIT = 0x4806;
constexpr uint64_t GUEST_FS_LIMIT = 0x4808;
constexpr uint64_t GUEST_GS_LIMIT = 0x480A;
constexpr uint64_t GUEST_LDTR_LIMIT = 0x480C;
constexpr uint64_t GUEST_TR_LIMIT = 0x480E;
constexpr uint64_t GUEST_GDTR_LIMIT = 0x4810;
constexpr uint64_t GUEST_IDTR_LIMIT = 0x4812;

// Guest state fields - 32-bit access rights
constexpr uint64_t GUEST_ES_ACCESS_RIGHTS = 0x4814;
constexpr uint64_t GUEST_CS_ACCESS_RIGHTS = 0x4816;
constexpr uint64_t GUEST_SS_ACCESS_RIGHTS = 0x4818;
constexpr uint64_t GUEST_DS_ACCESS_RIGHTS = 0x481A;
constexpr uint64_t GUEST_FS_ACCESS_RIGHTS = 0x481C;
constexpr uint64_t GUEST_GS_ACCESS_RIGHTS = 0x481E;
constexpr uint64_t GUEST_LDTR_ACCESS_RIGHTS = 0x4820;
constexpr uint64_t GUEST_TR_ACCESS_RIGHTS = 0x4822;

// Guest state fields - Natural-width (64-bit) bases and registers
constexpr uint64_t GUEST_ES_BASE = 0x6806;
constexpr uint64_t GUEST_CS_BASE = 0x6808;
constexpr uint64_t GUEST_SS_BASE = 0x680A;
constexpr uint64_t GUEST_DS_BASE = 0x680C;
constexpr uint64_t GUEST_FS_BASE = 0x680E;
constexpr uint64_t GUEST_GS_BASE = 0x6810;
constexpr uint64_t GUEST_LDTR_BASE = 0x6812;
constexpr uint64_t GUEST_TR_BASE = 0x6814;
constexpr uint64_t GUEST_GDTR_BASE = 0x6816;
constexpr uint64_t GUEST_IDTR_BASE = 0x6818;
constexpr uint64_t GUEST_RSP = 0x681C;
constexpr uint64_t GUEST_RIP = 0x681E;
constexpr uint64_t GUEST_RFLAGS = 0x6820;

// Guest control registers (natural-width)
constexpr uint64_t GUEST_CR0 = 0x6800;
constexpr uint64_t GUEST_CR3 = 0x6802;
constexpr uint64_t GUEST_CR4 = 0x6804;

// Host state fields - 16-bit selectors
constexpr uint64_t HOST_ES_SELECTOR = 0x0C00;
constexpr uint64_t HOST_CS_SELECTOR = 0x0C02;
constexpr uint64_t HOST_SS_SELECTOR = 0x0C04;
constexpr uint64_t HOST_DS_SELECTOR = 0x0C06;
constexpr uint64_t HOST_FS_SELECTOR = 0x0C08;
constexpr uint64_t HOST_GS_SELECTOR = 0x0C0A;
constexpr uint64_t HOST_TR_SELECTOR = 0x0C0C;

// Host control registers (natural-width)
constexpr uint64_t HOST_CR0 = 0x6C00;
constexpr uint64_t HOST_CR3 = 0x6C02;
constexpr uint64_t HOST_CR4 = 0x6C04;

// Host base addresses (natural-width)
constexpr uint64_t HOST_FS_BASE = 0x6C06;
constexpr uint64_t HOST_GS_BASE = 0x6C08;
constexpr uint64_t HOST_TR_BASE = 0x6C0A;
constexpr uint64_t HOST_GDTR_BASE = 0x6C0C;
constexpr uint64_t HOST_IDTR_BASE = 0x6C0E;

// Host registers (natural-width)
constexpr uint64_t HOST_RSP = 0x6C14;
constexpr uint64_t HOST_RIP = 0x6C16;

// VM-execution controls (32-bit)
constexpr uint64_t PIN_BASED_VM_EXEC_CONTROL = 0x4000;
constexpr uint64_t PROC_BASED_VM_EXEC_CONTROL = 0x4002;
constexpr uint64_t EXCEPTION_BITMAP = 0x4004;
constexpr uint64_t SECONDARY_VM_EXEC_CONTROL = 0x401E;

// VMCS link pointer
constexpr uint64_t VMCS_LINK_POINTER = 0x2800;

// VM-exit controls
constexpr uint64_t VM_EXIT_CONTROLS = 0x400C;

// VM-entry controls
constexpr uint64_t VM_ENTRY_CONTROLS = 0x4012;

// VM-exit information fields
constexpr uint64_t VM_EXIT_REASON = 0x4402;
constexpr uint64_t EXIT_QUALIFICATION = 0x6400;
constexpr uint64_t VM_INSTRUCTION_ERROR = 0x4400;

class VMX {
public:
    static bool is_enabled();
    static bool enable();
    static bool check_feature_control();
    static bool test_vmx();
    
private:
    static uint64_t read_msr(uint32_t msr);
    static void write_msr(uint32_t msr, uint64_t value);
};

#endif // VMX_H
