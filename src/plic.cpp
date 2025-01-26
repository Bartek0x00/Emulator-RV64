#include <exception>
#include "plic.hpp"

using namespace Emulator;

uint64_t Plic::load(uint64_t addr, uint64_t len)
{
	if (addr >= PRIORITY_BASE &&
		addr <= PRIORITY_BASE + PRIORITY_SIZE)
	{
		return priority[(addr - PRIORITY_BASE) / 4];
	}
	else if (addr >= PENDING_BASE &&
		addr <= PENDING_BASE + PENDING_SIZE)
	{
		return pending[(addr - PENDING_BASE) / 4];
	}
	else if (addr >= ENABLE_BASE &&
		addr <= ENABLE_BASE + ENABLE_SIZE)
	{
		return enable[(addr - ENABLE_BASE) / 4];
	}
	else if (addr >= TRESHOLD_CLAIM_BASE &&
		addr <= TRESHOLD_CLAIM_BASE + TRESHOLD_CLAIM_SIZE)
	{
		uint64_t ctx = (addr - TRESHOLD_CLAIM_BASE) / 0x1000ULL;
		uint64_t off = addr - (TRESHOLD_CLAIM_BASE + 0x1000ULL * ctx);

		if (off == 0)
			return treshold[ctx];
		else if (off == 4)
			return claim[ctx];
	}

	return 0;
}

void Plic::store(uint64_t addr, uint64_t value, uint64_t len)
{
	if (addr >= PRIORITY_BASE &&
		addr <= PRIORITY_BASE + PRIORITY_SIZE)
	{
		priority[(addr - PRIORITY_BASE) / 4] = value;
	}
	else if (addr >= PENDING_BASE &&
		addr <= PENDING_BASE + PENDING_SIZE)
	{
		pending[(addr - PENDING_BASE) / 4] = value;
	}
	else if (addr >= ENABLE_BASE &&
		addr <= ENABLE_BASE + ENABLE_SIZE)
	{
		enable[(addr - ENABLE_BASE) / 4] = value;
	}
	else if (addr >= TRESHOLD_CLAIM_BASE &&
		addr <= TRESHOLD_CLAIM_BASE + TRESHOLD_CLAIM_SIZE)
	{
		uint64_t ctx = (addr - TRESHOLD_CLAIM_BASE) / 0x1000ULL;
		uint64_t off = addr - (TRESHOLD_CLAIM_BASE + 0x1000ULL * ctx);

		if (off == 0)
			treshold[ctx] = value;
		else if (off == 4)
			clear_pending(value);
	}
}

void Plic::dump(void) const
{
	error<FAIL>(
		"################################\n"
		"#  Device: PLIC                #\n"
		"################################",
		"\n# base: ", base,
		"\n# size: ", size,
		"\n################################\n"
	);
}
