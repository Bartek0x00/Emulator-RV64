#pragma once

#include <exception>
#include <iostream>
#include <utility>

namespace Emulator {
	enum Error : uint64_t {
        INFO = 0,
        WARN = 1,
        FAIL = 2,
    };
	
	constexpr const char *color[3] = {
		"\033[0m",
		"\033[1;38;5;1m",
		"\033[1;38;5;3m"
	};

	template<Error E, typename... Args>
	inline void error(Args&&... args)
	{
		using namespace std;

		cerr << color[E] << hex;
		(cerr << ... << forward<Args>(args));
		cerr << color[0] << '\n';

		if constexpr (E == FAIL)
			throw runtime_error("FATAL");
	}
}
