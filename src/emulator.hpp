#pragma once

namespace Emulator {
	class Cpu;

	void run(void);

	enum Error {
		INFO,
		WARN,
		FAIL,
	};

	template<Error E>
	inline void error(const char *msg);

	template<>
	inline void error<INFO>(const char *msg)
	{
		fprintf(stderr, "%s\n", msg);
	}

	template<>
	inline void error<WARN>(const char *msg)
	{
		fprintf(stderr, "\033[38;5;3m%s\033[38;5;0m\n", msg);
	}

	template<>
	inline void error<FAIL>(const char *msg)
	{
		fprintf(stderr, "\033[38;5;1m%s\033[38;5;0m\n", msg);
		exit(EXIT_FAILURE);
	}
};
