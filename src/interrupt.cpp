#include "registers.hpp"
#include "common.hpp"
#include "gpu.hpp"
#include "virtio.hpp"
#include "plic.hpp"
#include "cpu.hpp"
#include "interrupt.hpp"

using namespace Emulator;

std::string_view Interrupt::get_name(void)
{
    switch (current) {
    case USER_SOFTWARE: 		return "UserSoftware";
    case SUPERVISOR_SOFTWARE:	return "SupervisorSoftware";
    case MACHINE_SOFTWARE: 		return "MachineSoftware";
    case USER_TIMER: 			return "UserTimer";
    case SUPERVISOR_TIMER: 		return "SupervisorTimer";
    case MACHINE_TIMER: 		return "MachineTimer";
    case USER_EXTERNAL: 		return "UserExternal";
    case SUPERVISOR_EXTERNAL: 	return "SupervisorExternal";
    case MACHINE_EXTERNAL: 		return "MachineExternal";
    default: 					return "UnknownInterrupt";
    }
}

void Interrupt::get_pending(void)
{
    switch (cpu->mode) {
    case Cpu::Mode::MACHINE:
    {
        uint64_t mie = read_bit(
            cpu->csr_regs.load(CRegs::Address::MSTATUS), 
            CRegs::Mstatus::MIE
        );

        if (!mie && !cpu->sleep) {
            current = Interrupt::NONE;
			return;
		}
        break;
    }

    case Cpu::Mode::SUPERVISOR:
    {
        uint64_t sie = read_bit(
            cpu->csr_regs.load(CRegs::Address::SSTATUS), 
            CRegs::Sstatus::SIE
        );

        if (!sie && !cpu->sleep) {
            current = Interrupt::NONE;
        	return;
		}
		break;
    }

    default: break;
    }

#ifndef EMU_DEBUG
    const uint32_t *irqn = 0;
	
	Gpu *gpu = static_cast<Gpu*>(
		bus->get(DeviceName::GPU)
	);
	if (gpu)
    	irqn = gpu->interrupting();
	
	Virtio *virtio = static_cast<Virtio*>(
		bus->get(DeviceName::VIRTIO)
	);
	if (virtio)
    	irqn = virtio->interrupting();

    if (irqn) {
		Plic *plic = static_cast<Plic*>(
			bus->get(DeviceName::PLIC)
		);
		if (plic)
        	plic->update_pending(*irqn);
        
		cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::SEIP_BIT, 
                1
            )
        );
    }
#endif

    uint64_t mie = cpu->csr_regs.load(CRegs::Address::MIE);
    uint64_t mip = cpu->csr_regs.load(CRegs::Address::MIP);
    uint64_t pending = mie & mip;

    if (pending == 0) {
        current = Interrupt::NONE;
		return;
	}
    
    if (pending & CRegs::Mask::MEIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::MEIP_BIT, 
                0
            )
        );
        current = Interrupt::MACHINE_EXTERNAL;
		return;
    }

    if (pending & CRegs::Mask::MSIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::MSIP_BIT, 
                0
            )    
        );
        current = Interrupt::MACHINE_SOFTWARE;
    	return;
	}
    
    if (pending & CRegs::Mask::MTIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::MTIP_BIT, 
                0
            )
        );
        current = Interrupt::MACHINE_TIMER;
		return;
    }
    
    if (pending & CRegs::Mask::SEIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::SEIP_BIT, 
                0
            )   
        );
        current = Interrupt::SUPERVISOR_EXTERNAL;
		return;
    }
    
    if (pending & CRegs::Mask::SSIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::SSIP_BIT, 
                0
            )
        );
        current = Interrupt::SUPERVISOR_SOFTWARE;
		return;
    }
    
    if (pending & CRegs::Mask::STIP) {
        cpu->csr_regs.store(
            CRegs::Address::MIP, 
            write_bit(
                cpu->csr_regs.load(CRegs::Address::MIP),
                CRegs::Mask::STIP_BIT, 
                0
            )
        );
        current = Interrupt::SUPERVISOR_TIMER;
		return;
    }

    current = Interrupt::NONE;
}

void Interrupt::process(void)
{
    cpu->sleep = false;
    uint64_t pc = cpu->pc;
    uint64_t mode = cpu->mode;

    bool mideleg_flag = 
        (cpu->csr_regs.load(CRegs::Address::MIDELEG) >> current) & 1;

    if (current == MACHINE_TIMER)
        mideleg_flag = false;

    if (mideleg_flag && (mode == Cpu::Mode::USER || mode == Cpu::Mode::SUPERVISOR)) {
        cpu->mode = Cpu::Mode::SUPERVISOR;

        uint64_t stvec_val = cpu->csr_regs.load(CRegs::Address::STVEC);
        uint64_t vt_off = 0;

        if (stvec_val & 1)
            vt_off = current * 4;

        cpu->pc = (stvec_val & ~3ULL) + vt_off;
        cpu->csr_regs.store(CRegs::Address::SEPC, (pc & ~1ULL));
        cpu->csr_regs.store(CRegs::Address::SCAUSE, (1ULL << 63ULL) | current);
        cpu->csr_regs.store(CRegs::Address::STVAL, 0);

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
        uint64_t vt_off = 0;

        if (mtvec_val & 1)
            vt_off = current * 4;

        cpu->pc = (mtvec_val & ~3ULL) + vt_off;
        cpu->csr_regs.store(CRegs::Address::MEPC, (pc & ~1ULL));
        cpu->csr_regs.store(CRegs::Address::MCAUSE, (1ULL << 63ULL) | current);
        cpu->csr_regs.store(CRegs::Address::MTVAL, 0);

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
                12, 11,
                mode
            )
        );
    }
}
