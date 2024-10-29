#pragma once

#include <cstdint>
#include "errors.hpp"

namespace Emulator {

	template<typename T, uint64_t U>
    class Registers {
	private:
    	T regs[U] = {0};

    public:
		template<typename T>
    	inline T& operator[](uint64_t index)
		{
			if (index < U)
				return regs[index];
				
			error<FAIL>(
				"Register access out of bounds: ", 
				index
			);
			
			return static_cast<T&>(0);
		}
	};
};
