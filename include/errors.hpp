#pragma once

#include <exception>

namespace Emulator {
	enum class Error : uint64_t {
        INFO = 0,
        WARN,
        FAIL,
		EXIT
    };
	
	constexpr const char *COLOR_NONE = "\033[0m";
	constexpr const char *COLOR_WARN = "\033[1;38;5;1m";
	constexpr const char *COLOR_FAIL = "\033[1;38;5;3m";

	template<Error E, typename... Args>
	constexpr inline void error(Args&&... args);

	template<typename... Args>
	constexpr inline void error<INFO>(Args&&... args)
	{
		(std::cerr << ... << std::forward<Args>(args)) << '\n';
	}

	template<typename... Args>
	constexpr inline void error<WARN>(Args&&... args)
	{
		std::cerr << COLOR_WARN;

		(std::cerr << ... << std::forward<Args>(args)) << '\n';

		std::cerr << COLOR_NONE;
	}

	[[noreturn]]
	template<typename... Args>
	constexpr inline void error<FAIL>(Args&&... args)
	{
		std::cerr << COLOR_FAIL;

		(std::cerr << ... << std::forward<Args>(args)) << std::endl;

		std::cerr << COLOR_NONE;

		throw std::exception();
	}
};
