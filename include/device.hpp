#pragma once

#include <cstdint>
#include <string_view>

namespace Emulator {
	class Device {	
	public:
		const uint64_t base;
		const uint64_t size;
		const std::string_view name;

		virtual inline uint64_t load(uint64_t addr) = 0;
		virtual inline void store(uint64_t addr, uint64_t value) = 0;

		virtual ~Device() = default;
	};
};
