#include "cpu.hpp"

using namespace Emulator;

Cpu cpu;

void Cpu::dump_regs(void)
{
	error<INFO>(
		"################################\n"
		"#  Integer Registers           #\n"
		"################################"
	);

	for (int i = 0; i < 32; i++)
		error<INFO>("\n# ", i, " ", int_regs[i]);
	
	error<INFO>(
		"\n#"
		"################################\n"
		"#  Floating-Point Registers    #\n"
		"################################"
	);

	for (int i = 0; i < 32; i++)
		error<INFO>("\n# ", i, " ", flt_regs[i]);
	
	error<INFO>(
		"\n#"
		"################################"
		"\n# pc: ", pc,
		"\n# wfi: ", sleep,
		"\n###############################"
	);
}

void Cpu::run(void)
{
	while (true) {
		csr_regs.store(
			CRegs::Address::CYCLE,
			csr_regs.load(
				CRegs::Address::CYCLE
			) + 1
		);

		bus["CLINT"].tick();
		bus["GPU"].tick();

		interrupt.get_pending();

		if (interrupt.current != Interrupt::NONE) {
			error<INFO>(
				"################################\n"
				"# Interrupt                    #\n"
				"################################"
				"\n# current: ", interrupt.get_name(),
				"\n################################\n"
			);

			interrupt.process();
		}

		uint32_t insn_size = cycle();

		if (exception.current != Exception::NONE) {
			error<INFO>(
				"################################\n"
				"# Exception                    #\n"
				"################################"
				"\n# current: ", exception.get_name(),
				"\n# value: ", exception.value,
				"\n################################\n"
			);

			exception.process();
			clear_exception();

			continue;;
		}

		pc += insn_size;
	}
}

uint32_t Cpu::cycle(void)
{
	int_regs[IRegs::zero] = 0;

	if (sleep)
		return 0;
	
	uint32_t insn = mmu.fetch(pc);

	if (exception.current != Exception::NONE)
		return 4;
	
	Decoder decoder = Decoder(insn);

	error<INFO>(
		"################################\n"
		"# At Address ", pc
		"\n################################\n"
	);

	decoder.dump();

	if (!decoder.insn) {
		set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return 4;
	}

	return Instruction::execute(decoder);
}

