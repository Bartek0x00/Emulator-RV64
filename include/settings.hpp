#pragma once

#include "common.hpp"

#ifndef FONT_SIZE
#define FONT_SIZE 16
#endif

#ifndef DRAM_BASE
#define DRAM_BASE 0x80000000U
#endif

#ifndef RAM_SIZE
#define RAM_SIZE BYTE_SIZE<MIB>(64)
#endif

#ifndef KERNEL_OFFSET
#define KERNEL_OFFSET 0x200000U
#endif
