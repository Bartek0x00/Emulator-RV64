#include <cstdint>
#include "instruction.hpp"

using namespace Emulator;
using namespace Emulator::Instruction;
using Emulator::Decoder;

uint64_t execute(Decoder decoder)
{
	if (decoder.insn == 0) {
		cpu.set_exception(Exception::ILLEGAL_INSTRUCTION, decoder.insn);
		error<FAIL>("Illegal instruction");
	}
	
	if (decoder.size() == 2) {
		switch (static_cast<OpcodeType>(decoder.opcode_c())) {
		case OpcodeType::COMPRESSED_QUANDRANT0: 
			C::quadrant0(decoder); 
			break;
		case OpcodeType::COMPRESSED_QUANDRANT1:
			C::quadrant1(decoder);
			break;
		case OpcodeType::COMPRESSED_QUANDRANT2: 
			C::quadrant2(decoder); 
			break;
		default:
			cpu.set_exception(Exception::ILLEGAL_INSTRUCTION, decoder.insn);
			break;
		}
	} else {
		switch (static_cast<OpcodeType>(decoder.opcode())) {
		case OpcodeType::LOAD: 
			LD::funct3(decoder); 
			break;
		case OpcodeType::FENCE:
			F::funct3(decoder);
			break;
		case OpcodeType::I:
			I::funct3(decoder);
			break;
		case OpcodeType::S:
			ST::funct3(decoder);
			break;
		case OpcodeType::R:
			R::funct3(decoder);
			break;
		case OpcodeType::B:
			B::funct3(decoder);
			break;
		case OpcodeType::FL:
			FD::fl(decoder);
			break;
		case OpcodeType::FS:
			FD::fs(decoder);
			break;
		case OpcodeType::FMADD:
			FD::fmadd(decoder);
			break;
		case OpcodeType::FMSUB:
			FD::fmsub(decoder);
			break;
		case OpcodeType::FNMADD:
			FD::fnmadd(decoder);
			break;
		case OpcodeType::FNMSUB:
			FD::fnmsub(decoder);
			break;
		case OpcodeType::FOTHER:
			FD::fother(decoder);
			break;
		case OpcodeType::ATOMIC:
			A::funct3(decoder);
			break;
		case OpcodeType::I64:
			I64::funct3(decoder);
			break;
		case OpcodeType::R64:
			R64::funct3(decoder);
			break;
		case OpcodeType::AUIPC:
			AUIPC::auipc(decoder);
			break;
		case OpcodeType::LUI:
			LUI::lui(decoder);
			break;
		case OpcodeType::JAL:
			JAL::jal(decoder);
			break;
		case OpcodeType::JALR:
			JALR::jalr(decoder);
			break;
		case OpcodeType::CSR:
			CSR::funct3(decoder);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION, 
				decoder.insn
			):
			break;
		}
	}

	return decoder.size();
}

namespace LD {

static void funct3(Decoder decoder)
{
	switch (static_cast<LdType>(decoder.funct3()) {
	case LdType::LB:
		LD::lb(decoder);
		break;
	case LdType::LH:
		LD::lh(decoder);
		break;
	case LdType::LW:
		LD::lw(decoder);
		break;
	case LdType::LD:
		LD::ld(decoder);
		break;
	case LdType::LBU:
		LD::lbu(decoder);
		break;
	case LdType::LHU:
		LD::lhu(decoder);
		break;
	case LdType::LWU:
		LD::lwu(decoder);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn()
		);
		break;
	}
}
	
static void LD::lb(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rs1] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 8)
		);
}

static void LD::lh(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rs1] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 16)
		);
}

static void LD::lw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rs1] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 32)
		);
}

static void LD::ld(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rs1] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 64)
		);
}

static void LD::lbu(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rd] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 8)
		);
}

static void LD::lhu(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rd] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 16)
		);
}

static void LD::lwu(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();

	uint64_t addr = cpu.int_regs[rd] + imm;

	if (cpu.exc_val == Exception::NONE)
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 64)
		);
}

}; // namespace LD

namespace A {

static void funct3(Decoder decoder)
{
	switch (static_cast<AType>(decoder.funct3())) {
	case AType::AMOW:

		if (cpu.int_regs[decoder.rs1()] % 4)
			cpu.set_exception(
				Exception::INSTRUCTION_ADDRESS_MISALIGNED, 
				decoder.insn
			);

		switch (static_cast<AType>(decoder.funct5())) {
		case AType::ADD: 	amoaddw(decoder);	break;
		case AType::SWAP: 	amoswapw(decoder); 	break;
		case AType::LR: 	lrw(decoder); 		break;
		case AType::SC: 	scw(decoder); 		break;
		case AType::XOR: 	amoxorw(decoder); 	break;
		case AType::OR: 	amoorw(decoder); 	break;
		case AType::AND: 	amoandw(decoder); 	break;
		case AType::MIN: 	amominw(decoder); 	break;
		case AType::MAX: 	amomaxw(decoder); 	break;
		case AType::MINU: 	amominuw(decoder);	break;
		case AType::MAXU: 	amomaxuw(decoder); 	break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	
	case AType::AMOD:
		
		if (cpu.int_regs[decoder.rs1()] % 8)
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);

		switch (static_cast<AType>(decoder.funct5())) {
		case AType::ADD:	amoaddd(decoder); 	break;
		case AType::SWAP:	amoswapd(decoder);	break;
		case AType::LR:		lrd(decoder); 		break;
		case AType::SC: 	scd(decoder); 		break;
		case AType::XOR: 	amoxord(decoder); 	break;
		case AType::OR: 	amoord(decoder); 	break;
		case AType::AND: 	amoandd(decoder); 	break;
		case AType::MIN: 	amomind(decoder); 	break;
		case AType::MAX: 	amomaxd(decoder); 	break;
		case AType::MINU: 	amominud(decoder); 	break;
		case AType::MAXU: 	amomaxud(decoder); 	break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
		break;
	}
}

static void amoaddw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	int32_t res = val1 + val2;

	mmu.store(addr, val2, 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoswapw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, val2, 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void lrw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];
	int32_t val1 = mmu.load(addr, 32);
	
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
	cpu.reservations.insert(addr);
}

static void scw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];
	uint32_t val2 = cpu.int_regs[rs2];

	if (cpu.reservations.contains(addr)) {
		cpu.reservations.erase(addr);
		mmu.store(addr, val2, 32);
		cpu.int_regs[rd] = 0;
	} else {
		cpu.int_regs[rd] = 1;
	}
}

static void amoxorw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 ^ val2), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoorw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 | val2), 32);
	cpu.int_regs[rd] = static_cast<int32_t>(val1);
}

static void amoandw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 & val2), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amominw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::min(val1, val2)), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amomaxw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int32_t val1 = mmu.load(addr, 32);
	int32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::max(val1, val2)), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amominuw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.regs[rs1];
	
	uint32_t val1 = mmu.load(addr, 32);
	uint32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, std::min(val1, val2), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amomaxuw(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	uint32_t val1 = mmu.load(addr, 32);
	uint32_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, std::max(val1, val2), 32);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoaddd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	int64_t result = val1 + val2;

	mmu.store(addr, result, 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoswapd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, val2, 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void lrd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];
	int64_t val1 = mmu.load(addr, 64);

	cpu.int_regs[rd] = static_cast<int64_t>(val1);
	cpu.reservations.insert(addr);
}

static void scd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];
	uint64_t val2 = cpu.int_regs[rs2];

	if (cpu.reservations.contains(addr)) {
		cpu.reservations.erase(addr);
		mmu.store(addr, val2, 64);
		cpu.int_regs[rd] = 0;
	} else {
		cpu.int_regs[rd] = 1;
	}
}

static void amoxord(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 ^ val2), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoord(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 | val2), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amoandd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(val1 & val2), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amomind(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::min(val1, val2)), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amomaxd(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	int64_t val1 = mmu.load(addr, 64);
	int64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::max(val1, val2)), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amominud(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	uint64_t val1 = mmu.load(addr, 64);
	uint64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::min(val1, val2)), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

static void amomaxud(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rd = decoder.rd();

	uint64_t addr = cpu.int_regs[rs1];

	uint64_t val1 = mmu.load(addr, 64);
	uint64_t val2 = cpu.int_regs[rs2];

	mmu.store(addr, static_cast<int64_t>(std::max(val1, val2)), 64);
	cpu.int_regs[rd] = static_cast<int64_t>(val1);
}

}; // namespace A

namespace 
