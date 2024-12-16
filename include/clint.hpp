#pragma once

#include "device.hpp"

namespace Emulator {
	class Clint : public Device {
	private:
		uint32_t msip = 0;
		constexpr uint64_t MSIP_BASE = 0x2000000;
		constexpr uint64_t MSIP_SIZE = 4;

		uint64_t mtimecmp = 0;
		constexpr uint64_t MTIMECMP_BASE = 0x2004000;
		constexpr uint64_t MTIMECMP_SIZE = 8;

		uint64_t mtime = 0;
		constexpr uint64_t MTIME_BASE = 0x200bff8;
		constexpr uint64_t MTIME_SIZE = 8;
	
	public:
		explicit inline Clint(void) : \
			base(0x2000000),
			size(0x10000),
			name("CLINT")
		
		uint64_t load(uint64_t addr, uint64_t len) override;
		void store(uint64_t addr, uint64_t value, uint64_t len) override;
		void dump(void) const override;
		void tick(void) override;
	};
};
