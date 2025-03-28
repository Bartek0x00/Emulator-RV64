#include "common.hpp"
#include "mmu.hpp"

using namespace Emulator;

uint64_t Mmu::load(uint64_t addr, uint64_t len)
{
	uint64_t p_addr = translate(addr, AccessType::LOAD);

	if (cpu->exception.current != Exception::NONE)
		return 0;
	
	uint64_t value = bus->load(p_addr, len);
	return value;
}

void Mmu::store(uint64_t addr, uint64_t value, uint64_t len)
{
	uint64_t p_addr = translate(addr, AccessType::STORE);

	if (cpu->exception.current != Exception::NONE)
		return;

	bus->store(p_addr, value, len);
}

uint64_t Mmu::fetch(uint64_t addr, uint64_t len)
{
	uint64_t p_addr = translate(addr, AccessType::INSTRUCTION);
	if (cpu->exception.current != Exception::NONE)
		return 0;
	
	uint64_t value = bus->load(p_addr, len);
	if (cpu->exception.current == Exception::LOAD_ACCESS_FAULT)
		cpu->exception.current = Exception::INSTRUCTION_ACCESS_FAULT;
	
	return value;
}

void Mmu::update(void)
{
	uint64_t satp = cpu->csr_regs.load(CRegs::Address::SATP);

	mppn = read_bits(satp, 43, 0) << 12ULL;
	mode = read_bits(satp, 63, 60);

	flush_tlb();
}

bool Mmu::fetch_pte(uint64_t addr, uint64_t access_type, uint64_t cpu_mode, TLBEntry& entry)
{
	std::array<uint64_t, 5> vpn = get_vpn(addr);
	uint64_t levels = get_levels();
	constexpr uint64_t PTE_SIZE = 8;

	uint64_t tmp = mppn;
	int64_t i = levels - 1;

	for (; i >= 0; i--) {
		entry.pte_addr = tmp + vpn[i] * PTE_SIZE;
		entry.pte = bus->load(entry.pte_addr, 64);
		
		entry.is_read = (entry.pte >> PteValue::READ) & 1;
		entry.is_write = (entry.pte >> PteValue::WRITE) & 1;
		entry.is_execute = (entry.pte >> PteValue::EXECUTE) & 1;

		bool valid = (entry.pte >> PteValue::VALID) & 1;

		if (!valid || (!entry.is_read && entry.is_write)) {
			set_cpu_error(addr, access_type);
			return false;
		}

		if (entry.is_read || entry.is_execute)
			break;

		tmp = ((entry.pte >> 10ULL) & 0xfffffffffffULL) * PAGE_SIZE;
	}

	if (i < 0) {
		set_cpu_error(addr, access_type);
		return false;
	}

	std::array<uint64_t, 5> ppn = get_ppn(entry.pte);

	for (int j = 0; j < i; j++)
		if (ppn[j]) {
			set_cpu_error(addr, access_type);
			return false;
		}

	entry.is_user = (entry.pte >> PteValue::USER) & 1;
	entry.is_accessed = (entry.pte >> PteValue::ACCESSED) & 1;
	entry.is_dirty = (entry.pte >> PteValue::DIRTY) & 1;

	if (i) {
		entry.phys_base = 0;

		for (uint64_t j = 0; j < i; j++)
			entry.phys_base |= vpn[j] << (12ULL + (j * 9ULL));

		for (uint64_t j = i; j < levels; j++)
			entry.phys_base |= ppn[j] << (12ULL + (j * 9ULL));
	} else
		entry.phys_base = ((entry.pte >> 10ULL) & 0xfffffffffffULL) << 12ULL;
	
	return true;
}

Mmu::TLBEntry *Mmu::get_tlb_entry(uint64_t addr, uint64_t access_type, uint64_t cpu_mode)
{
	uint64_t addr_masked = addr & ~0xfffULL;
	uint64_t oldest_tlb_age = 0;
	int oldest_tlb_index = 0;

	for (int i = 0; i < tlb_cache.size(); i++) {
		TLBEntry& entry = tlb_cache[i];

		if (addr_masked == entry.virt_base) {
			entry.age = 0;

			while (++i < tlb_cache.size())
				++tlb_cache[i].age;

			return &entry;
		}

		++entry.age;

		if (entry.age > oldest_tlb_age) {
			oldest_tlb_age = entry.age;
			oldest_tlb_index = i;
		}
	}

	TLBEntry& entry = tlb_cache[oldest_tlb_index];
	
	if (fetch_pte(addr, access_type, cpu_mode, entry)) {
		entry.virt_base = addr_masked;
		entry.age = 0;
		return &entry;
	}

	return nullptr;
}

uint64_t Mmu::translate(uint64_t addr, uint64_t access_type)
{
	if (mode == ModeValue::BARE)
		return addr;
	
	uint64_t cpu_mode = cpu->mode;
	
	uint64_t mprv = read_bit(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		),
		CRegs::Mstatus::MPRV
	);

	if (access_type != AccessType::INSTRUCTION && (mprv == 1))
	{
		uint64_t mpp = read_bits(
			cpu->csr_regs.load(
				CRegs::Address::MSTATUS
			), 
			12, 11
		);
		
		switch (mpp) {
		case Cpu::Mode::USER:
		case Cpu::Mode::SUPERVISOR:
		case Cpu::Mode::MACHINE:
			cpu_mode = mpp;
			break;
		default:
			cpu_mode = Cpu::Mode::INVALID;
			break;
		}
	}

	if (cpu_mode == Cpu::Mode::MACHINE)
		return addr;
	
	TLBEntry *entry = get_tlb_entry(addr, access_type, cpu_mode);
	if (!entry)
		return 0;
	
	bool mxr = read_bit(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		), 
		CRegs::Mstatus::MXR
	);
	bool sum = read_bit(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		), 
		CRegs::Mstatus::SUM
	);

	if ((!entry->is_read && entry->is_write && !entry->is_execute) ||
		(!entry->is_read && entry->is_write && entry->is_execute)) 
	{
		set_cpu_error(addr, access_type);
		return 0;
	}

	if (entry->is_user && 
		((cpu_mode != Cpu::Mode::USER) &&
		 (!sum || access_type == AccessType::INSTRUCTION)))
	{
		set_cpu_error(addr, access_type);
		return 0;
	}

	if (!entry->is_user && (cpu_mode != Cpu::Mode::SUPERVISOR)) {
		set_cpu_error(addr, access_type);
		return 0;
	}

	switch (access_type) {
	case AccessType::LOAD:
		if (!(entry->is_read || (entry->is_execute && mxr))) {
			set_cpu_error(addr, access_type);
			return 0;
		}
		break;
	case AccessType::STORE:
		if (!entry->is_write) {
			set_cpu_error(addr, access_type);
			return 0;
		}
		break;
	case AccessType::INSTRUCTION:
		if (!entry->is_execute) {
			set_cpu_error(addr, access_type);
			return 0;
		}
		break;
	}

	if (!entry->is_accessed || (access_type == AccessType::STORE && !entry->is_dirty)) {
		entry->pte |= 1ULL << PteValue::ACCESSED;
		entry->is_accessed = true;

		if (access_type == AccessType::STORE) {
			entry->pte |= 1ULL << PteValue::DIRTY;
			entry->is_dirty = true;
		}

		bus->store(entry->pte_addr, entry->pte, 64);
	}

	return entry->phys_base | (addr & 0xfffULL);
}

namespace Emulator {
	std::unique_ptr<Mmu> mmu;
};
