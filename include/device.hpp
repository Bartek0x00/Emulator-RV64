#pragma once

#include <cstdint>
#include <memory>

namespace Emulator {
    struct DeviceNode {
        uint64_t base;
        uint64_t size;
        std::unique_ptr<Device> dev;
    };

    class Device {
    public:
        virtual bool load(uint64_t addr, uint64_t bytes, uint8_t *buffer) = 0;
        virtual bool store(uint64_t addr, uint64_t bytes, const uint8_t *buffer) = 0;
        virtual void reset(void) = 0;
        virtual ~Device(void) = 0;
    };
};