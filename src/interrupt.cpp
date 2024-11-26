#include "interrupt.hpp"

using namespace Emulator;

string_view Interrupt::get_name(InterruptValue int_value)
{
    switch (value) {
    case USER_SOFTWARE: return "UserSoftware";
    case SUPERVISOR_SOFTWARE: return "SupervisorSoftware";
    case MACHINE_SOFTWARE: return "MachineSoftware";
    case USER_TIMER: return "UserTimer";
    case SUPERVISOR_TIMER: return "SupervisorTimer";
    case MACHINE_TIMER: return "MachineTimer";
    case USER_EXTERNAL: return "UserExternal";
    case SUPERVISOR_EXTERNAL: return "SupervisorExternal";
    case MACHINE_EXTERNAL: return "MachineExternal";
    default: return "UnknownInterrupt";
    }
}

//TODO
InterruptValue Interrupt::get_pending(void)
{
    switch (cpu.mode) {
    case Cpu::Mode::MACHINE:
        if (!read_bit(cpu.csr_regs.load(CRegs::Address::MSTATUS), CRegs::Mask::MIE) && !cpu.sleep)
            return Interrupt::NONE;
        break;

    case Cpu::Mode::SUPERVISOR:
        if (!read_bit(cpu.csr_regs.load(CRegs::Address::SSTATUS), CRegs::Mask::SIE) && !cpu.sleep)
            return Interrupt::NONE;
        break;

    default: break;
    }

    uint32_t *irqn;

    if ((irqn = bus.find("VIRTIO").interrupting()))
        break;

    if ((irqn = bus.find("GPU").interrupting()))
        break;

    if (irqn) {
        cpu.find("PLIC").update_pending(*irqn);
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::SEIP_BIT, 1);
    }

    uint64_t mie = cpu.csr_regs.load(CRegs::Address::MIE);
    uint64_t mip = cpu.csr_regs.load(CRegs::Address::MIP);
    uint64_t pending = mie & mip;

    if (pending == 0)
        return NONE;
    
    if (pending & CRegs::Mask::MEIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::MEIP_BIT, 0);
        return MACHINE_EXTERNAL;
    } else if (pending & CRegs::Mask::MSIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::MSIP_BIT, 0);
        return MACHINE_SOFTWARE;
    } else if (pending & CRegs::Mask::MTIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::MTIP_BIT, 0);
        return MACHINE_TIMER;
    } else if (pending & CRegs::Mask::SEIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::SEIP_BIT, 0);
        return SUPERVISOR_EXTERNAL;
    } else if (pending & CRegs::Mask::SSIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::SSIP_BIT, 0);
        return SUPERVISOR_SOFTWARE;
    } else if (pending & CRegs::Mask::STIP) {
        cpu.csr_regs.write_bit(CRegs::Address::MIP, CRegs::Mask::STIP_BIT, 0);
        return SUPERVISOR_TIMER;
    }

    return NONE;
}

//TODO
void Interrupt::process(InterruptValue int_value)
{
    cpu.sleep = false;
    uint64_t pc = cpu.pc;
    Cpu::Mode mode = cpu.mode;
    bool mideleg_flag = 
        (cpu.csr_regs.load(CRegs::Address::MIDELEG) >> int_val) & 1;

    if (int_val == MACHINE_TIMER)
        mideleg_flag = false;

    if (mideleg_flag && 
        (mode == Cpu::Mode::USER || mode == Cpu::Mode::SUPERVISOR)
    ) {
        cpu.mode = Cpu::Mode::SUPERVISOR;

        uint64_t stvec_val = cpu.cregs.load(CRegs::Address::STVEC);
        uint64_t vt_offset = 0;

        if (stvec_val & 1)
            vt_offset = int_val * 4;

        cpu.pc = (stvec_val & ~3ULL) + vt_offset;
        cpu.csr_regs.store(CRegs::Address::SEPC, pc & ~1ULL);
        cpu.csr_regs.store(CRegs::Address::SCAUSE, (1ULL << 63ULL) | int_val);
        cpu.csr_regs.store(CRegs::Address::STVAL, 0);
        cpu.csr_regs.write_bit_sstatus(CRegs::Mask::SSTATUSBit::SPIE,
                                    cpu.cregs.read_bit_sstatus(CRegs::Mask::SSTATUSBit::SIE));
        cpu.csr_regs.write_bit_sstatus(CRegs::Mask::SSTATUSBit::SIE, 0);
        cpu.csr_regs.write_bit_sstatus(CRegs::Mask::SSTATUSBit::SPP, mode);
    
    } else {
        cpu.mode = Cpu::Mode::MACHINE;

        uint64_t mtvec_val = cpu.csr_regs.load(CRegs::Address::MTVEC);
        uint64_t vt_offset = 0;

        if (mtvec_val & 1)
            vt_offset = int_val * 4;

        cpu.pc = (mtvec_val & ~3ULL) + vt_offset;
        cpu.csr_regs.store(CRegs::Address::MEPC, pc & ~1ULL);
        cpu.csr_regs.store(CRegs::Address::MCAUSE, (1ULL << 63ULL) | int_val);
        cpu.csr_regs.store(CRegs::Address::MTVAL, 0);
        cpu.csr_regs.write_bit_mstatus(CRegs::Mask::MSTATUSBit::MPIE,
                                    cpu.cregs.read_bit_mstatus(CRegs::Mask::MSTATUSBit::MIE));
        cpu.csr_regs.write_bit_mstatus(CRegs::Mask::MSTATUSBit::MIE, 0);
        cpu.csr_regs.write_mpp_mode(mode);
    }
}