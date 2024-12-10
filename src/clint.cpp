#include "clint.hpp"

using namespace Emulator;

uint64_t Clint::load(uint64_t addr)
{
	uint64_t reg_value = 0;
	uint64_t off = 0;

	if (addr >= MSIP_BASE &&
		addr <= MSIP_BASE + MSIP_SIZE)
	{
		reg_value = msip;
		off = addr - MSIP_BASE;
	}
	else if (addr >= MTIMECMP_BASE &&
		addr <= MTIMECMP_BASE + MTIMECMP_SIZE)
	{
		reg_value = mtimecmp;
		off = addr - MTIMECMP_BASE;
	}
	else if (addr >= MTIME_BASE &&
		addr <= MTIME_BASE + MTIME_SIZE)
	{
		reg_value = mtime;
		off = addr - MTIME_BASE;
	}

	return reg_value >> (off * 8ULL);
}

void Clint::store(uint64_t addr, uint64_t value)
{
	uint64_t reg_value = 0;
	uint64_t off = 0;

	if (addr >= MSIP_BASE &&
		addr <= MSIP_BASE + MSIP_SIZE)
	{
		reg_value = msip;
		off = addr - MSIP_BASE;
	}
	else if (addr >= MTIMECMP_BASE &&
		addr <= MTIMECMP_BASE + MTIMECMP_SIZE)
	{
		reg_value = mtimecmp;
		off = addr - MTIMECMP_BASE;
	}
	else if (addr >= MTIME_BASE &&
		addr <= MTIME_BASE + MTIME_SIZE)
	{
		reg_value = mtime;
		off = addr - MTIME_BASE;
	}

	reg_value = value;

	if (addr >= MSIP_BASE &&
		addr <= MSIP_BASE + MSIP_SIZE)
	{
		msip = reg_value;
	}
	else if (addr >= MTIMECMP_BASE &&
		addr <= MTIMECMP_BASE + MTIMECMP_SIZE)
	{
		mtimecmp = reg_value;
	}
	else if (addr >= MTIME_BASE &&
		addr <= MTIME_BASE + MTIME_SIZE)
	{
		mtime = reg_value;
	}
}

void Clint::dump(void) const
{
	error<INFO>(
		"################################\n"
		"#  Device: CLINT               #\n"
		"################################"
		"\n# base: ", base,
		"\n# size: ", size,
		"\n# msip: ", msip,
		"\n# mtimecmp: ", mtimecmp,
		"\n# mtime: ", mtime,
		"\n################################"
	);
}

void Clint::tick(void)
{
	mtime = get_milliseconds() * 1000;
	cpu.csr_regs.store(CRegs::Address::TIME, mtime);

	if (msip & 1)
		cpu.csr_regs.store(
			write_bit(
				cpu.csr_regs.load(CRegs::Address::MIP), 
				CRegs::Mask::MSIP_BIT, 1
			)
		);

	if (mtime >= mtimecmp)
		cpu.csr_regs.store(
			write_bit(
				cpu.csr_regs.load(CRegs::Address::MIP), 
				CRegs::Mask::MTIP_BIT, 1
			)
		);
	else
		cpu.csr_regs.store(
			write_bit(
				cpu.csr_regs.load(CRegs::Address::MIP), 
				CRegs::Mask::MTIP_BIT, 0
			)
		);
}
