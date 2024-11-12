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
    default: return "Unknown interrupt";
    }
}

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

    std::optional<uint32_t> irqn;

    if ((bus.virtio_blk_device != nullptr) && 
        (irqn = bus.virtio_blk_device->is_interrupting())
    ) [[unlikely]]
        break;

    if ((irqn = bus.gpu_device->is_interrupting())) [[unlikely]]
        break;

    if (irqn) [[unlikely]] {
        cpu.plic_device.update_pending(*irqn);

        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::SEIP_BIT, 1);
    }

    uint64_t mie = cpu.cregs.load(csr::Address::MIE);
    uint64_t mip = cpu.cregs.load(csr::Address::MIP);

    uint64_t pending = mie & mip;

    if (pending == 0) [[likely]]
    {
        return interrupt::Interrupt::None;
    }
    if (pending & csr::Mask::MEIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::MEIP_BIT, 0);
        return interrupt::Interrupt::MachineExternal;
    }
    else if (pending & csr::Mask::MSIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::MSIP_BIT, 0);
        return interrupt::Interrupt::MachineSoftware;
    }
    else if (pending & csr::Mask::MTIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::MTIP_BIT, 0);
        return interrupt::Interrupt::MachineTimer;
    }
    else if (pending & csr::Mask::SEIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::SEIP_BIT, 0);
        return interrupt::Interrupt::SupervisorExternal;
    }
    else if (pending & csr::Mask::SSIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::SSIP_BIT, 0);
        return interrupt::Interrupt::SupervisorSoftware;
    }
    else if (pending & csr::Mask::STIP)
    {
        cpu.cregs.write_bit(csr::Address::MIP, csr::Mask::STIP_BIT, 0);
        return interrupt::Interrupt::SupervisorTimer;
    }

    return interrupt::Interrupt::None;
}