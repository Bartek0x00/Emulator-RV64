#include <bit>
#include "clint.hpp"
#include "gpu.hpp"
#include "virtio.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "decoder.hpp"
#include "instruction.hpp"

using namespace Emulator;

void Cpu::dump_regs(void)
{
	error<INFO>(
		"################################\n"
		"#  Integer Registers           #\n"
		"################################"
	);

	for (int i = 0; i < 32; i++)
		error<INFO>("# ", i, " ", int_regs[i]);
	
	error<INFO>(
		"\n#"
		"################################\n"
		"#  Floating-Point Registers    #\n"
		"################################"
	);

	for (int i = 0; i < 32; i++)
		error<INFO>("# ", i, " ", flt_regs[i].d, " ", flt_regs[i].u64);
	
	error<INFO>(
		"\n#"
		"################################\n"
		"#  Control-Status Registers    #\n"
		"################################"
		"\n# mip: ", csr_regs.load(CRegs::Address::MIP),
		"\n# mie: ", csr_regs.load(CRegs::Address::MIE),
		"\n# msip: ", read_bit(csr_regs.load(CRegs::Address::MIP), CRegs::Mask::MSIP_BIT),
		"\n# mstatus: ", csr_regs.load(CRegs::Address::MSTATUS),
		"\n# sip: ", csr_regs.load(CRegs::Address::SIP),
		"\n# sie: ", csr_regs.load(CRegs::Address::SIE),
		"\n# ssip: ", read_bit(csr_regs.load(CRegs::Address::MIP), CRegs::Mask::SSIP_BIT),
		"\n# sstatus: ", csr_regs.load(CRegs::Address::SSTATUS),
		"\n"
	);
}

void Cpu::iterate(void)
{
	csr_regs.store(
		CRegs::Address::CYCLE,
		csr_regs.load(
			CRegs::Address::CYCLE
		) + 1
	);

#ifndef EMU_DEBUG
	Clint *clint = static_cast<Clint*>(
		bus->get(DeviceName::CLINT)
	);
	if (clint)
		clint->tick();
	
	Gpu *gpu = static_cast<Gpu*>(
		bus->get(DeviceName::GPU)
	);
	if (gpu)
		gpu->tick();
	
	Virtio *virtio = static_cast<Virtio*>(
		bus->get(DeviceName::VIRTIO)
	);
	if (virtio)
		virtio->tick();
#endif
	interrupt.get_pending();

	if (interrupt.current != Interrupt::NONE) {
	#ifdef EMU_DEBUG
		error<INFO>(
			"################################\n"
			"# Interrupt                    #\n"
			"################################"
			"\n# current: ", interrupt.get_name(),
			"\n################################\n"
		);
	#endif
		interrupt.process();
	}

	uint32_t insn_size = _iterate();

	if (exception.current != Exception::NONE) {
	#ifdef EMU_DEBUG
		error<INFO>(
			"################################\n"
			"# Exception                    #\n"
			"################################"
			"\n# current: ", exception.get_name(),
			"\n# value: ", exception.value,
			"\n################################\n"
		);
	#endif
		exception.process();
	#ifndef EMU_DEBUG
		clear_exception();
	#endif
		return;
	}
	
	pc += insn_size;
}

uint32_t Cpu::_iterate(void)
{
	int_regs[IRegs::zero] = 0;

	if (sleep)
		return 0;
	
	uint32_t insn = mmu->fetch(pc);

	if (exception.current != Exception::NONE)
		return 4;
	
	Decoder decoder = Decoder(insn);
#ifdef EMU_DEBUG
	error<INFO>(
		"################################\n"
		"# At Address ", pc,
		"\n################################"
	);

	decoder.dump();
#endif
	if (!decoder.insn) {
		set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return 4;
	}
	
	return Instruction::execute(decoder);
}

namespace Emulator {
	std::unique_ptr<Cpu> cpu;
};
