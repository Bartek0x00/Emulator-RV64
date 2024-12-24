#pragma once

#include <array>
#include "device.hpp"
#include "errors.hpp"

namespace Emulator {
	class Plic : public Device {
	private:		
		using Region<S> = std::array<uint32_t, S>;

		Region<1024> priority;
		constexpr uint64_t PRIORITY_BASE = 0xC000000;
		constexpr uint64_t PRIORITY_SIZE = 0xFFF;

		Region<32> pending;
		constexpr uint64_t PENDING_BASE = 0xC001000;
		constexpr uint64_t PENDING_SIZE = 0x7F;

		Region<64> enable;
		constexpr uint64_t ENABLE_BASE = 0xC002000;
		constexpr uint64_t ENABLE_SIZE = 0xFF;

		Region<2> treshold;
		Region<2> claim;
		constexpr uint64_t TRESHOLD_CLAIM_BASE = 0xC200000;
		constexpr uint64_t TRESHOLD_CLAIM_SIZE = 0x1007;
	
	public:
		explicit inline Plic(void) : 
			Device(0xC000000ULL, 0x208000ULL, "PLIC") {};

		inline bool is_enabled(uint64_t ctx, uint64_t irq)
		{
			uint64_t idx = (irq % 1024) / 32;
			uint64_t off = (irq % 1024) % 32;

			return ((enable[ctx * 32 + idx] >> off) & 1) == 1;
		}
		
		inline void update_claim(uint64_t irq)
		{
			if (is_enabled(1, irq) || irq == 0)
				claim[1] = irq;
		}
		
		inline void clear_pending(uint64_t irq)
		{
			pending[irq / 4] &= ~(1U << irq);
			update_claim(0);
		}

		inline void update_pending(uint64_t irq)
		{
			pending[irq / 4] |= 1U << irq;
			update_claim(irq);
		}
		
		uint64_t load(uint64_t addr, uint64_t len) override;	
		void store(uint64_t addr, uint64_t value, uint64_t len) override;
		void dump(void) const override;
	};
};
