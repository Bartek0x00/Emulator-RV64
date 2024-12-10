#include <dram.hpp>

using namespace Emulator;

uint64_t Dram::load(uint64_t addr)
{
    addr -= base;

    if (addr >= size)
        error<FAIL>("DRAM: Cannot access data at address: ", addr);

    return data[addr];
}

void Dram::store(uint64_t addr, uint64_t value)
{
    addr -= base;

    if (addr >= size)
        error<FAIL>("DRAM: Cannot access data at address: ", addr);
    
    data[addr] = value;
}

void Dram::dump(void) const
{
	error<INFO>(
		"################################\n"
		"#  Device: DRAM                #\n"
		"################################"
		"\n# base: ", base,
		"\n# size: ", size,
		"\n################################\n"
	);
}
