#pragma once

#include <cstdint>
#include <chrono>

namespace Emulator {
	enum MemSize {
		KIB,
		MIB,
		GIB
	};

	template<MemSize S>
	constexpr inline uint64_t size(uint64_t num)
	{
		switch (S) {
		case KIB:
			return 1024	* num;
		case MIB:
			return 1024 * 1024 * num;
		case GIB:
			return 1024 * 1024 * 1024 * num;
		}
	}

	inline uint64_t get_milliseconds(void)
	{
		using namespace std::chrono;
		using clock = high_resolution_clock;
		
		static const time_point<clock> start_time = clock::now();
		time_point<clock> now = clock::now();

		return duration_cast<milliseconds>(
			now - start_time
		).count();
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
