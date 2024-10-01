#include <elf.h>
#include <cstdio>
#include <cstring>

using Emulator;

Memory::Memory(uint64_t mem_size) : size(mem_size), mem(mem_size, 0) {}

Memory::Memory(uint64_t mem_size, const char *filepath) : Memory(mem_size)
{
	std::ifstream file(filepath);

	if (!file.is_open())
		error<FAIL>("Cannot open the image file");

	
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