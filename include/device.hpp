#pragma once

#include <cstdint>
#include <string_view>

namespace Emulator {
	class Device {	
	public:
		const uint64_t base;
		const uint64_t size;
		const std::string_view name;

		virtual uint64_t load(uint64_t addr) = 0;
		virtual void store(uint64_t addr, uint64_t value) = 0;
		virtual void dump(void) const = 0;
		virtual void tick(void);

		virtual ~Device() = default;
	};
};
