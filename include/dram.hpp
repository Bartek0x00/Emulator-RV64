#pragma once

#include <cstdint>
#include <vector>
#include "errors.hpp"
#include "device.hpp"

namespace Emulator {
	class Dram : public Device {
	private:
		std::vector<uint8_t> data;

	public:
		explicit inline Dram(uint64_t base, uint64_t size) : 
			Device(base, size, "DRAM")
		{
			data.reserve(size);	
		}

		explicit inline Dram(
			uint64_t base,
			uint64_t size,
			std::vector<uint8_t> _data
		) : Dram(base, size), data(std::move(_data)) {};
		
		inline void copy(std::vector<uint8_t>& img,
			uint64_t off)
		{
			std::memcpy(data.data() + off, img.data(), img.size());
		}

		uint64_t load(uint64_t addr, uint64_t len) override;
		void store(uint64_t addr, uint64_t value, uint64_t len) override;
		void dump(void) const override;
	};
};
