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
		explicit inline Dram(
			uint64_t _base, 
			uint64_t _size,
		) : base(_base), size(_size), name("DRAM") 
		{
			data.reserve(size);	
		}

		explicit inline Dram(
			uint64_t _base,
			uint64_t _size,
			std::vector<uint8_t> _data
		) : Dram(_base, _size), data(std::move(_data)) {};
		
		uint64_t load(uint64_t addr) override;
		void store(uint64_t addr, uint64_t value) override;
		void dump(void) const override;
	};
};
