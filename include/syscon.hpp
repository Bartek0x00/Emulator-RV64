#pragma once

#include "device.hpp"

namespace Emulator {
	class Syscon : public Device {
	private:
		constexpr uint64_t POWEROFF_BASE = 0x5555;
		constexpr uint64_t REBOOT_BASE = 0x7777;
	
	public:
		explicit Syscon(void) : 
			Device(
				POWEROFF_BASE, 
				REBOOT_BASE - POWEROFF_BASE, 
				"SYSCON"
			) {};

		uint64_t load(uint64_t addr, uint64_t len) override;
		void store(uint64_t addr, uint64_t value, uint64_t len) override;
		void dump(void) const override;
	};
};
