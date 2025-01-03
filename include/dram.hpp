#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include "errors.hpp"
#include "device.hpp"

namespace Emulator {
	class Dram : public Device {
	private:
		std::vector<uint8_t> data;

	public:
		explicit Dram(uint64_t _base, uint64_t _size, std::vector<uint8_t> _data = {});

		void copy(std::vector<uint8_t>& img, uint64_t off);
		uint64_t load(uint64_t addr, uint64_t len) override;
		void store(uint64_t addr, uint64_t value, uint64_t len) override;
		void dump(void) const override;
	};
};
