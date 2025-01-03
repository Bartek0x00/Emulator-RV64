#include <algorithm>
#include <exception>
#include "bus.hpp"
#include "errors.hpp"

using namespace Emulator;

Device *Bus::get(uint64_t addr) const
{
	for (const std::unique_ptr<Device>& uptr_device : devices) {
		Device *device = uptr_device.get();
		if (!device)
			continue;

		if (addr >= device->base && 
			addr < device->base + device->size)
		{
			return device;
		}
	}

	return nullptr;
}

uint64_t Bus::load(uint64_t addr, uint64_t len)
{
	Device *device = get(addr);
	
	if (device)
		return device->load(addr, len);
	
	cpu->set_exception(
		Exception::LOAD_ACCESS_FAULT
	);
	return 0;
}

void Bus::store(uint64_t addr, uint64_t value, uint64_t len)
{
	Device *device = get(addr);

	if (device)
		device->store(addr, value, len);
	else
		cpu->set_exception(
			Exception::STORE_ACCESS_FAULT
		);
}

void Bus::dump(void) const
{
	error<INFO>(
		"\t################################\n"
		"\t#      Devices dump start      #\n"
		"\t################################\n"
	);

	for (const std::unique_ptr<Device>& device : devices) {
		device->dump();
	}

	error<INFO>(
		"\t################################\n"
		"\t#       Devices dump end       #\n"
		"\t################################\n"
	);
}

namespace Emulator {
	std::unique_ptr<Bus> bus;
};
