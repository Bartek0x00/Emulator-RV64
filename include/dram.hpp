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
		
		inline uint64_t load(uint64_t addr) override
		{
			addr -= base;

			if (addr < size)
				return data[addr];
				
			error<FAIL>(
				name,
				": Cannot access data at address: ",
				addr
			);

			return 0;
		}

		inline void store(uint64_t addr, uint64_t value) override
		{
			addr -= base;

			if (addr < size)
				data[addr] = value;

			error<FAIL>(
				name,
				": Cannot access data at address: ",
				addr
			);
		}
	};
};
