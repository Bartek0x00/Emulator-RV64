#pragma once

#include <iostream>
#include <exception>
#include "cpu.hpp"
#include "bus.hpp"

class Emulator {
private:
	Bus bus;
	Cpu cpu;
	
	bool RVC = true;

	enum Error {
        INFO = 0,
        WARN,
        FAIL,
    };

	constexpr const char *error_color[3] = {
		"\033[0m", "\033[1;38;5;1m", "\033[1;38;5;3m"
	};
	
	void run(void);

public:

	explicit Emulator(const char *filepath);

	template<Error E>
	constexpr inline void error()
	{
		std::cerr << error_color[E] << value << error_color[0] << '\n';
		
		if (E == FAIL)
			throw std::exception();
	}
};
