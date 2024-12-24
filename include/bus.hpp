#pragma once

#include <vector>
#include <memory>
#include <string_view>
#include "device.hpp"

namespace Emulator {
	template<typename T>
	concept InheritedDevice = std::is_base_of<Device, T>::value;
	
	class Bus {
	private:
		std::vector<std::unique_ptr<Device>> devices;

	public:
		explicit Bus(void) = default;
		
		Device& operator[](uint64_t addr) const;
		Device& operator[](std::string_view name) const;
		void dump(void) const;
		
		template<InheritedDevice T, typename... Args>
		inline void add(Args&&... args)
		{
			devices.emplace_back(
				std::make_unique<T>(
					std::forward<Args>(args)...
				)
			);
		}
		
		inline uint64_t load(uint64_t addr, uint64_t len)
		{
			return operator[](addr).load(addr, len);
		}

		inline void store(uint64_t addr, uint64_t value, uint64_t len)
		{
			operator[](addr).store(addr, value, len);
		}
	};
};
