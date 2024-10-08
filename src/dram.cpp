#include <elf.h>
#include <cstdio>
#include <cstring>
#include "emulator.hpp"

using namespace Emulator;

Dram::Dram(const char *input_file, uint64_t mem_size = 0)
{
	std::ifstream file(filepath);

	if (!file.is_open())
		error<FAIL>("Cannot open the image file");

	
	mem.reserve(size);


}

bool Dram::load(uint64_t addr, uint64_t bytes, uint8_t *buffer)
{
	if (addr + bytes <= size) {
		memcpy(buffer, &mem[addr], bytes);
		return true;
	}

	return false;
}

bool Dram::store(uint64_t addr, uint64_t bytes, const uint8_t *buffer)
{
	if (addr + bytes <= size) {
		memcpy(&mem[addr], buffer, bytes);
		return true;
	}

	return false;
}