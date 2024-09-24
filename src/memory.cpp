#include <elf.h>
#include <cstdio>
#include <cstring>
#include "errors.hpp"

using namespace Emulator;

Memory::Memory(uint64_t mem_size) : size(mem_size), mem(mem_size, 0) {}

Memory::Memory(uint64_t mem_size, const char *file) : Memory(mem_size)
{
	FILE *file = fopen(file);
	if (!file)
		error<FAIL, STRING>("Cannot open file:", file);

	fseek(file, 0, SEEK_END);
	uint64_t file_size = ftell(file);
	rewind(file); 

	if (file_size > mem_size) {
		mem.reserve(mem_size);
		fread(mem.data(), 1, mem_size, file);
	} else {
		mem.reserve(file_size);
		fread(mem.data(), 1, file_size, file);
	}

	fclose(file);
}

bool Memory::load(uint64_t addr, uint64_t bytes, uint8_t *buffer)
{
	if (addr + bytes <= size) {
		memcpy(buffer, &mem[addr], bytes);
		return true;
	}

	return false;
}

bool Memory::store(uint64_t addr, uint64_t bytes, const uint8_t *buffer)
{
	if (addr + bytes <= size) {
		memcpy(&mem[addr], buffer, bytes);
		return true;
	}

	return false;
}