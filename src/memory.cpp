#include "memory.hpp"

using namespace Emulator;

bool Memory::load_img(const char *filename)
{
    FILE *file = fopen(filename, "r+");
    if (!file)
        return false;

    fread(memory, SIZE, 1, file);

    fclose(file);
}