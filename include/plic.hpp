#pragma once

#include <array>
#include "device.hpp"
#include "errors.hpp"

namespace Emulator {
	class Plic : public Device {
	private:
		
		using Region<S> = std::array<uint32_t, S>;

		Region<1024> priority;
		constexpr priority_base = 0xC000000;
		constexpr priority_size = 0xFFF;

		Region<32> pending;
		constexpr pending_base = 0xC001000;
		constexpr pending_size = 0x7F;

		Region<64> enable;
		constexpr enable_base = 0xC002000;
		constexpr enable_size = 0xFF;

		Region<2> treshold;
		Region<2> claim;
		constexpr treshold_claim_base = 0xC200000;
		constexpr treshold_claim_size = 0x1007;
	
	public:
		explicit inline Plic(void) : \
			base(0xC000000ULL), 
			size(0x208000ULL),
			name("PLIC") {};

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
		
		inline uint64_t load(uint64_t addr) override
		{
			if (addr >= priority_base && 
				addr <= priority_base + priority_size)
			{
				return priority[(addr - priority_base) / 4];
			}

			if (addr >= pending_base &&
				addr <= pending_base + pending_size)
			{
				return pending[(addr - pending_base) / 4];
			}

			if (addr >= enable_base &&
				addr <= enable_base + enable_size)
			{
				return enable[(addr - enable_base) / 4];
			}

			if (addr >= treshold_claim_base &&
				addr <= treshold_claim_base + treshold_claim_size)
			{
				uint64_t ctx = (addr - treshold_claim_base) / 0x1000ULL;
				uint64_t off = addr - (treshold_claim_base + 0x1000ULL * ctx);
				
				if (off == 0)
					return treshold[ctx];
				else if (off == 4)
					return claim[ctx];
			}

			error<FAIL>(
				name,
				": Cannot access region at the address: ",
				addr
			);
			
			return 0;
		}
		
		inline void store(uint64_t addr, uint64_t value)
		{
			if (addr >= priority_base &&
				addr <= priority_base + priority_size)
			{
				priority[(addr - priority_base) / 4] = value;
			}

			if (addr >= pending_base &&
				addr <= pending_base + pending_size)
			{
				pending[(addr - pending_base) / 4] = value;
			}

			if (addr >= enable_base &&
				addr <= enable_base + enable_size)
			{
				enable[(addr - enable_base) / 4] = value;
			}

			if (addr >= treshold_claim_base &&
				addr <= treshold_claim_base + treshold_claim_size)
			{
				uint64_t ctx = (addr - treshold_claim_base) / 0x1000ULL;
				uint64_t off = addr - (treshold_claim_base + 0x1000ULL * ctx);

				if (off == 0)
					treshold[ctx] = value;
				else if (off == 4)
					clear_pending(value);
			}
		}
	};
};
