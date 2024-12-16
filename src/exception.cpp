#include "exception.hpp"

using namespace Emulator;

std::string_view Exception::get_name(ExceptionValue exc_val)
{
    switch (exc_val) {
    case INSTRUCTION_ADDRESS_MISALIGNED:	return "InstructionAddressMisaligned";
    case INSTRUCTION_ACCESS_FAULT: 			return "InstructionAccessFault";
    case ILLEGAL_INSTRUCTION: 				return "IllegalInstruction";
    case BREAKPOINT: 						return "Breakpoint";
    case LOAD_ADDRESS_MISALIGNED: 			return "LoadAddressMisaligned";
    case LOAD_ACCESS_FAULT: 				return "LoadAccessFault";
    case STORE_ADDRESS_MiSALIGNED: 			return "StoreAddressMisaligned";
    case STORE_ACCESS_FAULT: 				return "StoreAccessFault";
    case ECALL_UMODE: 						return "ECallUmode";
    case ECALL_SMODE: 						return "ECallSmode";
    case ECALL_MMODE: 						return "ECallMmode";
    case INSTRUCTION_PAGE_FAULT: 			return "InstructionPageFault";
    case LOAD_PAGE_FAULT: 					return "LoadPageFault";
    case STORE_PAGE_FAULT: 					return "StorePageFault";
    default: 								return "UnknownException";
    }
}

void Exception::process(void)
{
    cpu.sleep = false;
    uint64_t pc = cpu.pc;
    Cpu::Mode mode = cpu.mode;

    bool medeleg_flag = 
        (cpu.csr_regs.load(CRegs::Address::MEDELEG) >> cpu.exc_val) & 1;

    if (medeleg_flag && 
        (mode == Cpu::Mode::USER || mode == Cpu::Mode::SUPERVISOR)
    ) {
        cpu.mode = Cpu::Mode::SUPERVISOR;
        uint64_t stvec_val = cpu.csr_regs.load(CRegs::Address::STVEC);
        cpu.pc = stvec_val & ~3ULL;

        cpu.csr_regs.store(CRegs::Address::SEPC, (pc & ~1ULL));
        cpu.csr_regs.store(CRegs::Address::SCAUSE, cpu.exc_val);
        cpu.csr_regs.store(Cregs::Address::STVAL, cpu.exc_data);

        uint64_t sie = read_bit(
            cpu.csr_regs.load(CRegs::Address::SSTATUS), 
            CRegs::Mask::SIE
        );
        cpu.csr_regs.store(
            CRegs::Address::SSTATUS, 
            write_bit(
                cpu.csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Mask::SPIE, 
                sie
            ) 
        );
        cpu.csr_regs.store(
            CRegs::Address::SSTATUS,
            write_bit(
                cpu.csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Mask::SIE,
                0
            )
        );
        cpu.csr_regs.store(
            CRegs::Address::SSTATUS,
            write_bit(
                cpu.csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Mask::SPP, 
                mode
            )
        );
    } else {
        cpu.mode = Cpu::Mode::MACHINE;
        uint64_t mtvec_val = cpu.csr_regs.load(CRegs::Address::MTVEC);
        cpu.pc = mtvec_val & ~3ULL;

        cpu.csr_regs.store(CRegs::Address::MEPC, (pc & ~1ULL));
        cpu.csr_regs.store(CRegs::Address::MCAUSE, cpu.exc_val);
        cpu.csr_regs.store(CRegs::Address::MTVAL, cpu.exc_data);
        
        uint64_t mie = read_bit(
            cpu.csr_regs.load(CRegs::Address::MSTATUS),
            CRegs::Mask::MIE
        );
        cpu.csr_regs.store(
            CRegs::Address::MSTATUS,
            write_bit(
                cpu.csr_regs.load(CRegs::Address::MSTATUS),
                CRegs::Mask::MPIE,
                mie
            )
        );
        cpu.csr_regs.store(
            CRegs::Address::MSTATUS, 
            write_bit(
                cpu.csr_regs.load(CRegs::Address::MSTATUS),
                CRegs::Mask::MIE, 
                0
            )
        );
        cpu.csr_regs.store(
            CRegs::Address::MSTATUS,
            write_bits(
                cpu.csr_regs.load(CRegs::Address::MSTATUS),
                12, 
                11, 
                mode
            )
        );
    }
}
