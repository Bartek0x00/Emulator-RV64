#pragma once

#include <cstdint>

namespace Emulator {
	inline uint64_t get_milliseconds(void)
	{
		using clock = std::chrono::high_resolution_clock;
		using cast = std::chrono::duration_cast
		using msec = std::chrono::miliseconds;

		static const auto start_time = clock::now();
		auto now = clock::now();

		return cast<msec>(now - start_time).count();
	}
	
	inline uint64_t read_bit(uint64_t value, uint64_t off)
	{
		return !!(value & (1ULL << off));
	}

	inline uint64_t read_bits(uint64_t value, 
		uint64_t upper, uint64_t lower)
	{
		uint64_t mask = (1ULL << (upper - lower + 1ULL)) - 1ULL;
		return (value >> lower) & mask;
	}

	inline uint64_t write_bit(uint64_t value, 
		uint64_t off, uint64_t bit_value)
	{
		if (bit_value)
			return value & ~(1ULL << off);
		else
			return value | (1ULL << off);
	}

	inline uint64_t write_bits(uint64_t value, uint64_t upper, 
		uint64_t lower, uint64_t bits_value)
	{
		uint64_t mask = (1ULL << (upper - lower + 1ULL)) - 1ULL;

		mask <<= lower;
		bits_value <<= lower;
		value &= ~mask;
		value |= bits_value;

		return value;
	}

	inline uint64_t align_up(uint64_t value, uint64_t alignment)
	{
		return ((value + alignment - 1) / alignment) * alignment;
	}
}
