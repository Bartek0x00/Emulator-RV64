#include <exception>
#include "emulator.hpp"

using namespace Emulator;

int main(int argc, char *argv[])
{
    if (argc < 2)
        error<FAIL>("Usage: rv64-emu [image]");

    try
        Emulator emulator(argv[1]);
    catch (const std::exception&)
        return 1;

    return 0;
}
