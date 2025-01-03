#pragma once

#include <cstdint>
#include <exception>
#include <iostream>
#include <utility>

namespace Emulator {
	enum Error : uint64_t {
		INFO = 0,
		WARN = 1,
		FAIL = 2
	};
	
	static constexpr const char *CLEAR  = "\033[0m";
	static constexpr const char *RED    = "\033[91m";
	static constexpr const char *GREEN  = "\033[92m";
	static constexpr const char *ORANGE = "\033[93m";

	template<Error E, typename... Args>
	inline void error(Args&&... args)
	{
		using namespace std;
		
		if constexpr (E == INFO) {
			cerr << hex;
			(cerr << ... << forward<Args>(args));
			cerr << '\n';
		}
		
		if constexpr (E == WARN) {
			cerr << ORANGE << hex;
			(cerr << ... << forward<Args>(args));
			cerr << CLEAR << '\n';
		}

		if constexpr (E == FAIL) {
			cerr << RED << hex;
			(cerr << ... << forward<Args>(args));
			cerr << CLEAR << '\n';
			throw runtime_error("FATAL");
		}
	}
}
