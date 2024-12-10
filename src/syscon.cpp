#include <exception>
#include "syscon.hpp"

using namespace Emulator;

uint64_t Syscon::load(uint64_t addr)
{
	return 0;
}

void Syscon::store(uint64_t addr, uint64_t value)
{
	if (value == POWEROFF_BASE || value == REBOOT_BASE)
		error<FAIL>();
}

void Syscon::dump(void) const
{
	error<INFO>(
		"################################\n"
		"#  Device: SYSCON              #\n"
		"################################"
		"\n# base: ", base,
		"\n# size: ", size,
		"\n################################\n"
	);
}
