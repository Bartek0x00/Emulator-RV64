#include <cstdlib>
#include <cstdio>

#pragma once

namespace Emulator {
    enum Error {
        INFO,
        WARN,
        FAIL,
    };

	#define CLR "\033[0m"
	#define RED "\033[1;38;5;1m"
	#define YEL "\033[1;38;5;3m"

	template<Error E>
	inline void error(const char *msg);

	template<>
	inline void error<INFO>(const char *msg)
	{
		fprintf(stderr, "INFO: %s\n", msg);
	}

	template<>
	inline void error<WARN>(const char *msg)
	{
		fprintf(
			stderr, "%sWARN: %s%s\n", 
			YEL, msg, CLR
		);
	}

	template<>
	inline void error<FAIL>(const char *msg)
	{
		fprintf(
			stderr, "%sFAIL: %s%s\n",
			RED, msg, CLR
		);

		exit(EXIT_FAILURE);
	}

    template<Error E>
    inline void error(const char *msg, double val);

	template<>
	inline void error<INFO>(const char *msg, double val)
	{
		fprintf(stderr, "INFO: %s%g\n", msg, val);
	}

    template<>
	inline void error<WARN>(const char *msg, double val)
	{
		fprintf(
			stderr, "%sWARN: %s%g%s\n", 
			YEL, msg, val, CLR
		);
	}

	template<>
	inline void error<FAIL>(const char *msg, double val)
	{
		fprintf(
			stderr, "%sFAIL: %s%g%s\n", 
			RED, msg, val, CLR
		);
		exit(EXIT_FAILURE);
	}
};
