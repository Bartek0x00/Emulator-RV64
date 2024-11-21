#include "common.hpp"
#include "mmu.hpp"

using namespace Emulator;

uint64_t Mmu::fetch(uint64_t addr)
{
	uint64_t p_addr = translate(addr, AccessType::INSTRUCTION);
	if (cpu.exc_val != exception::Exception::NONE)
		return 0;
	
	uint64_t value = bus.load(p_addr);
	if (cpu.exc_val == exception::Exception::LOAD_ACCESS_FAULT)
		cpu.exc_val = exception::Exception::INSTRUCTION_ACCESS_FAULT;
	
	return value;
}

uint64_t Mmu::load(uint64_t addr)
{
	uint64_t p_addr = translate(addr, AccessType::LOAD);
	if (cpu.exc_val != exception::Exception::NONE)
		return 0;
	
	return bus.load(p_addr);
}

void Mmu::store(uint64_t addr, uint64_t value)
{
	uint64_t p_addr = translate(addr, AccessType::STORE);
	if (cpu.exc_val != exception::Exception::NONE)
		return;
	
	bus.store(p_addr, value);
}

void Mmu::update(void)
{
	uint64_t satp = cpu.csr_regs[CRegs::Address::SATP];

	this->mppn = read_bits(satp, 43, 0) << 12ULL;
	this->mode = static_cast<ModeValue>(
		read_bits(satp, 63, 60)
	);

	flush_tlb();
}

bool Mmu::fetch_pte(uint64_t addr, AccessType access_type, Cpu::Mode cpu_mode, TLBEntry& entry)
{
	std::array<uint64_t, 5> vpn = get_vpn(addr);
	uint64_t levels = get_levels();
	constexpr uint64_t PTE_SIZE = 8;

	uint64_t tmp = this->mppn;
	int64_t i = levels - 1;

	for (; i >= 0; i--) {
		entry.pte_addr = tmp + vpn[i] * PTE_SIZE;
		entry.pte = bus.load(entry.pte_addr);
		
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

TLBEntry *Mmu::get_tlb_entry(uint64_t addr, AccessType access_type, Cpu::Mode cpu_mode)
{
	uint64_t addr_masked = addr & ~0xfffULL;
	uint64_t oldest_tlb_age = 0;
	int oldest_tlb_index = 0;

	for (int i = 0; i < this->tlb_cache.size(); i++) {
		TLBEntry& entry = tlb_cache[i];

		if (addr_masked == entry.virt_base) {
			entry.age = 0;

			while (++i < this->tlb_cache.size())
				++this->tlb_cache[i].age;

			return &entry;
		}
	}

	++entry.age;

	if (entry.age > oldest_tlb_age) {
		oldest_tlb_age = entry.age;
		oldest_tlb_index = i;
	}

	TLBEntry& entry = this->tlb_cache[oldest_tlb_index];
	if (fetch_pte(addr, access_type, cpu_mode, entry)) {
		entry.virt_base = addr_masked;
		entry.age = 0;
		return &entry;
	}

	return nullptr;
}

uint64_t Mmu::translate(uint64_t addr, AccessType access_type)
{
	if (this->mode == Mode::BARE)
		return addr;
	
	Cpu::Mode cpu_mode = cpu.mode;

	if (access_type != AccessType::INSTRUCTION &&
		read_bit(cpu.csr_regs.load(CRegs::Address::MSTATUS), CRegs::Mask::MPRV) == 1)
	{
		uint64_t mpp = read_bits(CRegs::Address::MSTATUS, 12, 11);
		
		switch (mpp) {
		case Cpu::Mode::USER:
		case Cpu::Mode::SUPERVISOR:
		case Cpu::Mode::MACHINE:
			cpu_mode = static_cast<Cpu::Mode>(mpp);
		default:
			cpu_mode = Cpu::Mode::INVALID;
		}
	}

	if (cpu_mode == Cpu::Mode::MACHINE)
		return addr;
	
	TLBEntry *entry = get_tlb_entry(addr, access_type, cpu_mode);
	if (!entry)
		return 0;
	
	bool mxr = read_bit(cpu.csr_regs.load(CRegs::Address::MSTATUS, CRegs::Mask::MXR);
	bool sum = read_bit(cpu.csr_regs.load(CRegs::Address::MSTATUS, CRegs::Mask::SUM);

	if ((!entry->is_read && entry->is_write && !entry->is_execute) ||
		(!entry->is_read && entry->is_write && entry->is_execute) 
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

	if (!entry->is_user && (cpu_mode != Cpu::Mode::SUPERVISOR) {
		set_cpu_error(addr, access_type);
		return 0;
	}

	switch (access_type) {
	case AccessType::LOAD:
		if (!(entry->is_read || (entry->is_execute && mxr)) {
			set_cpu_error(addr, access_type);
			return 0;
		}
	case AccessType::STORE:
		if (!entry->is_write) {
			set_cpu_error(addr, access_type);
			return 0;
		}
	case AccessType::INSTRUCTION:
		if (!entry->is_execute) {
			set_cpu_error(addr, access_type);
			return 0;
		}
	}

	if (!entry->is_accessed || (access_type == AccessType::STORE && !entry->is_dirty)) {
		entry->pte |= 1ULL << PteValue::ACCESSED;
		entry->is_accessed = true;

		if (access_type == AccessType::STORE) {
			entry->pte |= 1ULL << PteValue::DIRTY;
			entry->is_dirty = true;
		}

		bus.store(entry->pte_addr, entry->pte);
	}

	return entry->phys_base | (address & 0xfffULL);
}
