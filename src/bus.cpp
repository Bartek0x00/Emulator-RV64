#include <algorithm>
#include "bus.hpp"
#include "errors.hpp"

Device& Bus::operator[](uint64_t addr) const
{
	for (const Device& device : devices) {
		if (addr >= device.base && addr < device.base + device.size)
			return device;
	}

	error<FAIL>(
		"Cannot find the device at address: ", 
		addr
	);
}

Device& Bus::operator[](string_view name) const
{
	for (const Device& device : devices) {
		if (name == device.name)
			return device;
	}

	error<FAIL>(
		"Cannot find the device named: ",
		name
	);
}

void dump(void) const
{
	error<INFO>(
		"\t################################\n"
		"\t#      Devices dump start      #\n"
		"\t################################\n"
	);

	for (const Device& device : devices) {
		device.dump();
	}

	error<INFO>(
		"\t################################\n"
		"\t#       Devices dump end       #\n"
		"\t################################\n"
	);
}
