#pragma once

#ifndef DRAM_BASE
#define DRAM_BASE 0x80000000U
#endif

#ifndef RAM_SIZE
#define RAM_SIZE size<MIB>(64)
#endif

#ifndef KERNEL_OFFSET
#define KERNEL_OFFSET 0x200000U
#endif
