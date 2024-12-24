#include <algorithm>
#include "bus.hpp"
#include "errors.hpp"

using namespace Emulator;

Bus bus;

Device& Bus::operator[](uint64_t addr) const
{
	for (const std::unique_ptr<Device>& device : devices) {
		if (addr >= device->base && addr < device->base + device->size)
			return *device;
	}

	error<FAIL>(
		"Cannot find the device at address: ", 
		addr
	);

	return *devices[0];
}

Device& Bus::operator[](std::string_view name) const
{
	for (const std::unique_ptr<Device>& device : devices) {
		if (name == device->name)
			return *device;
	}

	error<FAIL>(
		"Cannot find the device named: ",
		name
	);

	return *devices[0];
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
