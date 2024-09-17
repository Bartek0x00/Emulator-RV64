#pragma once

namespace Emulator {
	class Registers;
	class Memory;

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
		fprintf(stderr, "%s%s%s\n", 
				ORANGE, msg, CLEAR);
	}

	template<>
	inline void error<FAIL>(const char *msg)
	{
		fprintf(stderr, "%s%s%s\n",
				RED, msg, CLEAR);
		exit(EXIT_FAILURE);
	}
};
