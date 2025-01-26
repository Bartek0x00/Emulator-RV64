#pragma once

#include "common.hpp"

using namespace Emulator;

static constexpr uint64_t FONT_SIZE = 16;
static constexpr uint64_t DRAM_BASE = 0x80000000ULL;
static constexpr uint64_t RAM_SIZE = BYTE_SIZE<MIB>(64);
static constexpr uint64_t KERNEL_OFFSET = 0x200000ULL;
