#pragma once

#include "bus.hpp"
#include <array>
#include <cstdint>

namespace Emulator {
	class Mmu {
    private:
		struct TLBEntry {
			uint64_t virt_base;
			uint64_t phys_base;
			uint64_t pte;
			uint64_t pte_addr;
			uint64_t age;
			bool is_dirty;
			bool is_accessed;
			bool is_read;
			bool is_write;
			bool is_execute;
			bool is_user;
		};
		
		enum class ModeValue : uint64_t {
			BARE = 0x00,
			SV39 = 0x08,
			SV48 = 0x09,
			SV57 = 0x0a
		};
		
		enum class PteValue : uint64_t {
			VALID = 0, READ,
			WRITE, EXECUTE,
			USER, GLOBAL,
			ACCESSED, DIRTY
		};

		enum class AccessType : uint64_t {
			LOAD = 0,
			STORE,
			INSTRUCTION
		};
		
		constexpr uint64_t page_size = 4096;
		
		ModeValue mode;
		std::array<TLBEntry, 4> tlb_cache;
		uint32_t mppn;

	public:
        explicit inline Mmu(void) : mode(Mode::BARE), tlb_cache{}
		{
			flush_tlb();
		}

		inline uint32_t get_levels(void)
		{
			switch (this->mode) {
			case Mode::SV39: return 3;
			case Mode::SV48: return 4;
			case Mode::SV57: return 5;
			default: return 0;
			}
		}

		inline void flush_tlb(void)
		{
			tlb_cache = {};

			for (TLBEntry& entry : tlb_cache)
				entry.age = (~0U);
		}
		
		inline std::array<uint64_t, 5> get_vpn(uint64_t addr)
		{
			std::array<uint64_t, 5> vpn{};

			for (int i = 0; i < get_levels(); i++)
				vpn[i] = (addr >> (12ULL + i * 9ULL)) & 0x1ffULL;

			return vpn;
		}
		
		inline std::array<uint64_t, 5> get_ppn(uint64_t pte)
		{
			std::array<uint64_t, 5> ppn{};
			
			for (int i = 0; i < get_levels(); i++)
				ppn[i] = (pte >> (10ULL + i * 9ULL)) & 0x1ffULL;

			switch (this->mode) {
			case Mode::SV39:
				ppn[2] = (pte >> 28) & 0x03ffffffULL;
				break;
			case Mode::SV48:
				ppn[3] = (pte >> 37) & 0x1ffffULL;
				break;
			case Mode::SV57:
				ppn[4] = (pte >> 48) & 0xffULL;
				break;
			default: break;
			}

			return ppn;
		}

		inline void set_cpu_error(uint64_t addr, AccessType access_type)
		{
			switch (access_type) {
			case AccessType::LOAD:
				cpu.set_exception(exception::Exception::LOAD_PAGE_FAULT, addr);
				break;
			case AccessType::STORE:
				cpu.set_exception(exception::Exception::STORE_PAGE_FAULT, addr);
				break;
			case AccessType::INSTRUCTION:
				cpu.set_exception(exception::Exception::INSTRUCTION_PAGE_FAULT, addr);
				break;
			}

			flush_tlb();
		}
		
		uint64_t translate(uint64_t addr, AccessType access_type);
		bool fetch_pte(uint64_t addr, AccessType access_type, 
			Cpu::Mode cpu_mode, TLBEntry& entry);
		TLBEntry *get_tlb_entry(uint64_t addr, AccessType access_type,
			Cpu::Mode cpu_mode);
		uint64_t fetch(uint64_t addr);
		uint64_t load(uint64_t addr);
		void store(uint64_t addr, uint64_t value);
		void update(void);
	};
};
