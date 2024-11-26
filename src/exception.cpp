#include "exception.hpp"

using namespace Emulator;

string_view Exception::get_name(ExceptionValue exc_val)
{
    switch (exc_val) {
    case INSTRUCTION_ADDRESS_MISALIGNED: return "InstructionAddressMisaligned";
    case INSTRUCTION_ACCESS_FAULT: return "InstructionAccessFault";
    case ILLEGAL_INSTRUCTION: return "IllegalInstruction";
    case BREAKPOINT: return "Breakpoint";
    case LOAD_ADDRESS_MISALIGNED: return "LoadAddressMisaligned";
    case LOAD_ACCESS_FAULT: return "LoadAccessFault";
    case STORE_ADDRESS_MiSALIGNED: return "StoreAddressMisaligned";
    case STORE_ACCESS_FAULT: return "StoreAccessFault";
    case ECALL_UMODE: return "ECallUmode";
    case ECALL_SMODE: return "ECallSmode";
    case ECALL_MMODE: return "ECallMmode";
    case INSTRUCTION_PAGE_FAULT: return "InstructionPageFault";
    case LOAD_PAGE_FAULT: return "LoadPageFault";
    case STORE_PAGE_FAULT: return "StorePageFault";
    default: return "UnknownException";
    }
}

//TODO
void Exception::process(void)
{
    cpu.sleep = false;
    uint64_t pc = cpu.pc;
    Cpu::Mode mode = cpu.mode;

    bool medeleg_flag = 
        (cpu.csr_regs.load(CRegs::Address::MEDELEG) >> cpu.exc_val) & 1;

    if (medeleg_flag && 
        (mode == Cpu::Mode::USER || mode == Cpu::Mode::SUPERVISOR)) 
    {
        cpu.mode = Cpu::Mode::SUPERVISOR;
        uint64_t stvec_val = cpu.csr_regs.load(CRegs::Address::STVEC);
        cpu.pc = stvec_val & ~3ULL;

        cpu.csr_regs.store(CRegs::Address::SEPC, pc & ~1ULL);
        cpu.csr_regs.store(CRegs::Address::SCAUSE, cpu.exc_val);
        cpu.csr_regs.store(Cregs::Address::STVAL, cpu.exc_data);
        write_bit(csr::Mask::SSTATUSBit::SPIE, cpu.cregs.read_bit_sstatus(csr::Mask::SSTATUSBit::SIE));
        cpu.csr_regs.write_bit_sstatus(csr::Mask::SSTATUSBit::SIE, 0);
        cpu.csr_regs.write_bit_sstatus(csr::Mask::SSTATUSBit::SPP, mode);
    
    } else {
        cpu.mode = Cpu::Mode::MACHINE;
        uint64_t mtvec_val = cpu.csr_regs.load(CRegs::Address::MTVEC);
        cpu.pc = mtvec_val & ~3ULL;

        cpu.csr_regs.store(CRegs::Address::MEPC, (pc & ~1ULL));
        cpu.csr_regs.store(CRegs::Address::MCAUSE, cpu.exc_val);
        cpu.csr_regs.store(CRegs::Address::MTVAL, cpu.exc_data);
        cpu.csr_regs.write_bit_mstatus(csr::Mask::MSTATUSBit::MPIE,
                                    cpu.cregs.read_bit_mstatus(csr::Mask::MSTATUSBit::MIE));
        cpu.csr_regs.write_bit_mstatus(csr::Mask::MSTATUSBit::MIE, 0);
        cpu.csr_regs.store(write_bits(CRegs::Address::MSTATUS, 12, 11, mode));

    }
}