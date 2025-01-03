#pragma once

#include <array>
#include <memory>
#include "cpu.hpp"
#include "device.hpp"

namespace Emulator {
	template<typename T>
	concept InheritedDevice = std::is_base_of<Device, T>::value;
		
	enum class DeviceName : size_t {
		CLINT = 0,
		DRAM,
		GPU,
		PLIC,
		SYSCON,
		VIRTIO,

		SIZE
	};
	
	class Bus {
	public:
		explicit constexpr inline Bus(void) = default;
	
		template<InheritedDevice T, DeviceName N, typename... Args>
		inline void add(Args&&... args)
		{
			devices[static_cast<size_t>(N)] = 
				std::make_unique<T>(
					std::forward<Args>(args)...
				);
		}		
		
		inline Device *get(DeviceName name) const
		{
			size_t index = static_cast<size_t>(name);

			return devices[index].get();
		}
	
		Device *get(uint64_t addr) const;
		uint64_t load(uint64_t addr, uint64_t len);
		void store(uint64_t addr, uint64_t value, uint64_t len);	
		void dump(void) const;
	
	private:
		std::array<
			std::unique_ptr<Device>, 
			static_cast<uint64_t>(DeviceName::SIZE)
		> devices;
	};

	extern std::unique_ptr<Bus> bus;
};
