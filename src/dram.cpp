#include <dram.hpp>

using namespace Emulator;

uint64_t Dram::load(uint64_t addr, uint64_t len)
{
    addr -= base;

    if (addr >= size)
        error<FAIL>("DRAM: Cannot access data at address: ", addr);

    switch (len) {
	case 8: return data[addr];
	case 16: return *reinterpret_cast<uint16_t*>(data.data() + addr);
	case 32: return *reinterpret_cast<uint32_t*>(data.data() + addr);
	case 64: return *reinterpret_cast<uint64_t*>(data.data() + addr);
	default: return 0;
	}
}

void Dram::store(uint64_t addr, uint64_t value, uint64_t len)
{
    addr -= base;

    if (addr >= size)
        error<FAIL>("DRAM: Cannot access data at address: ", addr);
    
    switch (len) {
	case 8: data[addr] = value; break;
	case 16: *reinterpret_cast<uint16_t*>(data.data() + addr) = value; break;
	case 32: *reinterpret_cast<uint32_t*>(data.data() + addr) = value; break;
	case 64: *reinterpret_cast<uint64_t*>(data.data() + addr) = value; break;
	default: break;
	}
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
