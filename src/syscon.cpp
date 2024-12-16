#include <exception>
#include "syscon.hpp"

using namespace Emulator;

uint64_t Syscon::load(uint64_t addr, uint64_t len)
{
	return 0;
}

void Syscon::store(uint64_t addr, uint64_t value, uint64_t len)
{
	if (value == POWEROFF_BASE || value == REBOOT_BASE)
		error<FAIL>("Shutdown...");
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
