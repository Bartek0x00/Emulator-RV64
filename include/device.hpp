#pragma once

#include <cstdint>
#include <string_view>

namespace Emulator {
	class Device {	
	public:
		const uint64_t base;
		const uint64_t size;
		
		virtual uint64_t load(uint64_t addr, uint64_t len) = 0;
		virtual void store(uint64_t addr, uint64_t value, uint64_t len) = 0;
		virtual void dump(void) const = 0;

		inline Device(uint64_t _base, uint64_t _size) :
			base(_base), size(_size) {};

		virtual ~Device() = default;
	};
};
