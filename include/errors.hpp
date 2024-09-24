#pragma once

#include <cstdlib>
#include <cstdio>

namespace Emulator {
    enum Error {
        INFO = 0,
        WARN,
        FAIL,
    };

	enum ErrorParamType {
		ULONG = 0,
		ILONG,
		DOUBLE,
		STRING
	};

	constexpr const char *fmt[4] = {
		"%lu", "%ld", "%g", "%s"
	};

	constexpr const char *color[3] = {
		"\033[0m", "\033[1;38;5;1m", "\033[1;38;5;3m"
	};

	template<Error E, ErrorParamType T>
	inline void error(const char *msg, T value)
	{
		fprintf(
			stderr, 
			color[E] "%s" fmt[T] color[0], 
			msg, value
		);

		if (E == 2)
			exit(EXIT_FAILURE);
	}

	template<Error E>
	inline void error(const char *msg)
	{
		error<E, STRING>(msg, "");
	}

	inline void error(const char *msg)
	{
		error<FAIL>(msg);
	}

};
