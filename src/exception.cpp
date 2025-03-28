#include "common.hpp"
#include "cpu.hpp"
#include "registers.hpp"
#include "exception.hpp"

using namespace Emulator;

std::string_view Exception::get_name(void)
{
    switch (current) {
    case INSTRUCTION_ADDRESS_MISALIGNED:	return "InstructionAddressMisaligned";
    case INSTRUCTION_ACCESS_FAULT: 			return "InstructionAccessFault";
    case ILLEGAL_INSTRUCTION: 				return "IllegalInstruction";
    case BREAKPOINT: 						return "Breakpoint";
    case LOAD_ADDRESS_MISALIGNED: 			return "LoadAddressMisaligned";
    case LOAD_ACCESS_FAULT: 				return "LoadAccessFault";
    case STORE_ADDRESS_MISALIGNED: 			return "StoreAddressMisaligned";
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
    cpu->sleep = false;
    uint64_t pc = cpu->pc;
    uint64_t mode = cpu->mode;

    bool medeleg_flag = 
        (cpu->csr_regs.load(CRegs::Address::MEDELEG) >> current) & 1;

    if (medeleg_flag && 
        (mode == Cpu::Mode::USER || mode == Cpu::Mode::SUPERVISOR)
    ) {
        cpu->mode = Cpu::Mode::SUPERVISOR;
        uint64_t stvec_val = cpu->csr_regs.load(CRegs::Address::STVEC);
        cpu->pc = stvec_val & ~3ULL;

        cpu->csr_regs.store(CRegs::Address::SEPC, (pc & ~1ULL));
        cpu->csr_regs.store(CRegs::Address::SCAUSE, current);
        cpu->csr_regs.store(CRegs::Address::STVAL, value);

        uint64_t sie = read_bit(
            cpu->csr_regs.load(CRegs::Address::SSTATUS), 
            CRegs::Sstatus::SIE
        );
        cpu->csr_regs.store(
            CRegs::Address::SSTATUS, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Sstatus::SPIE, 
                sie
            ) 
        );
        cpu->csr_regs.store(
            CRegs::Address::SSTATUS,
            write_bit(
                cpu->csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Sstatus::SIE,
                0
            )
        );
        cpu->csr_regs.store(
            CRegs::Address::SSTATUS,
            write_bit(
                cpu->csr_regs.load(CRegs::Address::SSTATUS),
                CRegs::Sstatus::SPP, 
                mode
            )
        );
    } else {
        cpu->mode = Cpu::Mode::MACHINE;
        uint64_t mtvec_val = cpu->csr_regs.load(CRegs::Address::MTVEC);
        cpu->pc = mtvec_val & ~3ULL;

        cpu->csr_regs.store(CRegs::Address::MEPC, (pc & ~1ULL));
        cpu->csr_regs.store(CRegs::Address::MCAUSE, current);
        cpu->csr_regs.store(CRegs::Address::MTVAL, value);
        
        uint64_t mie = read_bit(
            cpu->csr_regs.load(CRegs::Address::MSTATUS),
            CRegs::Mstatus::MIE
        );
        cpu->csr_regs.store(
            CRegs::Address::MSTATUS,
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MSTATUS),
                CRegs::Mstatus::MPIE,
                mie
            )
        );
        cpu->csr_regs.store(
            CRegs::Address::MSTATUS, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MSTATUS),
                CRegs::Mstatus::MIE, 
                0
            )
        );
        cpu->csr_regs.store(
            CRegs::Address::MSTATUS,
            write_bits(
                cpu->csr_regs.load(CRegs::Address::MSTATUS),
                12, 
                11, 
                mode
            )
        );
    }
}
