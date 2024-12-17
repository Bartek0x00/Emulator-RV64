#include <cstdint>
#include "instruction.hpp"

using namespace Emulator;
using Emulator::Decoder;

namespace Instruction {

uint64_t execute(Decoder decoder)
{
	if (decoder.insn == 0) {
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
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
			O::fence(decoder);
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
			O::auipc(decoder);
			break;
		case OpcodeType::LUI:
			O::lui(decoder);
			break;
		case OpcodeType::JAL:
			O::jal(decoder);
			break;
		case OpcodeType::JALR:
			O::jalr(decoder);
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

namespace C {

static void quadrant0(Decoder decoder)
{
	switch (static_cast<Q0>(decoder.funct3_c())) {
	case Q0::ADDI4SPN:
		C::addi4spn(decoder);
		break;
	case Q0::FLD:
		C::fld(decoder);
		break;
	case Q0::LW:
		C::lw(decoder);
		break;
	case Q0::LD:
		C::ld(decoder);
		break;
	case Q0::RESERVED:
		C::reserved(decoder);
		break;
	case Q0::FSD:
		C::fsd(decoder);
		break;
	case Q0::SW:
		C::sw(decoder);
		break;
	case Q0::SD:
		C::sd(decoder);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_EXCEPTION,
			decoder.insn
		);
	}
}

static void quadrant1(Decoder decoder)
{
	switch (static_cast<Q1>(decoder.funct3_c())) {
	case Q1::ADDI:
		C::addi(decoder);
		break;
	case Q1::ADDIW:
		C::addiw(decoder);
		break;
	case Q1::LI:
		C::li(decoder);
		break;
	case Q1::OP03:
		switch (static_cast<Q1>(decoder.rd())) {
		case Q1::NOP:
			break;
		case Q1::ADDI16SP:
			C::addi16sp(decoder);
			break;
		default:
			C::lui(decoder);
			break;
		}
	case Q1::OP04:
		switch (static_cast<Q1>(decoder.funct2_c())) {
		case Q1::SRLI:
			C::srli(decoder);
			break;
		case Q1::SRAI:
			C::srai(decoder);
			break;
		case Q1::ANDI:
			C::andi(decoder);
			break;
		case Q1::OP03:
			C::op3(decoder);
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
	case Q1::J:
		C::j(decoder);
		break;
	case Q1::BEQZ:
		C::beqz(decoder);
		break;
	case Q1::BNEZ:
		C::bnez(decoder);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

static void quadrant2(Decoder decoder)
{
	switch (static_cast<Q2>(decoder.funct3_c())) {
	case Q2::SLLI:
		C::slli(decoder);
		break;
	case Q2::FLDSP:
		C::fldsp(decoder);
		break;
	case Q2::LWSP:
		C::lwsp(decoder);
		break;
	case Q2::LDSP:
		C::ldsp(decoder);
		break;
	case Q2::OP4:
		C::op4(decoder);
		break;
	case Q2::FSDSP:
		C::fsdsp(decoder);
		break;
	case Q2::SWSP:
		C::swsp(decoder);
		break;
	case Q2::SDSP:
		C::sdsp(decoder);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
		break;
	}
}

static void addi4spn(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t imm = ((decoder.insn >> 1U) & 0x3c0U) | ((decoder.insn >> 7U) & 0x30U) |
                   ((decoder.insn >> 2U) & 0x08U) | ((decoder.insn >> 4U) & 0x04U);

    uint64_t val = cpu.int_regs[IRegs::sp] + imm;

    cpu.int_regs[rd] = val;
}

static void fld(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[rs1] + off;

    cpu.flt_regs[rd] = mmu.load(addr, 64);
}

static void lw(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0x40u) | ((decoder.insn >> 7U) & 0x38U) |
                      ((decoder.insn >> 4U) & 0x04U);

    uint64_t addr = cpu.int_regs[rs1] + off;

    cpu.int_regs[rd] = static_cast<int64_t>(mmu.load(addr, 32));
}

static void ld(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[rs1] + off;

    cpu.int_regs[rd] = mmu.load(addr, 64);
}

static void reserved(Decoder decoder)
{
    cpu.set_exception(
		Exception::ILLEGAL_INSTRUCTION, 
		decoder.insn
	);
}

static void fsd(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[rs1] + off;
    uint64_t rs1_bits = cpu.flt_regs[rs1];

    mmu.store(addr, rs1_bits, 64);
}

static void sw(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t rs2 = decoder.rs2_c();
    uint64_t off = ((decoder.insn << 1U) & 0x40U) | ((decoder.insn >> 7U) & 0x38U) |
                	((decoder.insn >> 4U) & 0x04U);

    uint64_t addr = cpu.int_regs[rs1] + off;
    uint32_t val = cpu.int_regs[rs2];

    mmu.store(addr, val, 32);
}

static void sd(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t rs2 = decoder.rs2_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[rs1] + off;
    uint64_t val = cpu.int_regs[rs2];

    mmu.store(addr, val, 64);
}

static void andi(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20U)
        imm = static_cast<int64_t>(imm | 0xc0U);

    cpu.int_regs[rd] &= imm;
}

static void op3(Decoder decoder)
{
	uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
	uint32_t val2 = (decoder.insn >> 5U) & 0x03U;

	uint64_t rd = decoder.rs1_c();
	uint64_t rs2 = decoder.rs2_c();

	if (val1) {
		switch (val2) {
		case 0:
			cpu.int_regs[rd] = static_cast<int64_t>(
				cpu.int_regs[rd] - cpu.int_regs[rs2]
			);
			break;
		case 1:
			cpu.int_regs[rd] = static_cast<int64_t>(
				cpu.int_regs[rd] + cpu.int_regs[rs2]
			);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
	} else {
		switch (val2) {
		case 0:
			cpu.int_regs[rd] -= cpu.int_regs[rs2];
			break;
		case 1:
			cpu.int_regs[rd] ^= cpu.int_regs[rs2];
			break;
		case 2:
			cpu.int_regs[rd] |= cpu.int_regs[rs2];
			break;
		case 3:
			cpu.int_regs[rd] &= cpu.int_regs[rs2];
			break;
		default:
			break;
		}
	}
}

static void addi(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = static_cast<int64_t>(imm | 0xc0U);

    cpu.int_regs[rd] += imm;
}

static void addiw(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = static_cast<int64_t>(imm | 0xc0U);

    cpu.int_regs[rd] = static_cast<int64_t>(cpu.int_regs[rd] + imm);
}

static void li(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = static_cast<int64_t>(imm | 0xc0U);

    cpu.int_regs[rd] = imm;
}

static void addi16sp(Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 3U) & 0x200U) | ((decoder.insn >> 2U) & 0x10U) |
                   ((decoder.insn << 1U) & 0x40U) | ((decoder.insn << 4U) & 0x180U) |
                   ((decoder.insn << 3U) & 0x20U);

    if (imm & 0x200)
        imm = static_cast<int64_t>(imm | 0xfc00U);

    cpu.int_regs[IRegs::sp] += imm;
}

static void lui(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn << 5U) & 0x20000U) | ((decoder.insn << 10U) & 0x1f000U);

    if (imm & 0x20000U)
        imm = static_cast<int64_t>(imm | 0xfffc0000U);

    cpu.int_regs[rd] = imm;
}

static void srli(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t shamt = decoder.shamt_c();

    cpu.int_regs[rd] >>= shamt;
}

static void srai(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t shamt = decoder.shamt_c();

    cpu.int_regs[rd] = static_cast<int64_t>(cpu.int_regs[rd]) >> shamt;
}

static void j(Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 1U) & 0x800U) | ((decoder.insn << 2U) & 0x400U) |
                   ((decoder.insn >> 1U) & 0x300U) | ((decoder.insn << 1U) & 0x80U) |
                   ((decoder.insn >> 1U) & 0x40U) | ((decoder.insn << 3U) & 0x20U) |
                   ((decoder.insn >> 7U) & 0x10U) | ((decoder.insn >> 2U) & 0xeU);

    if (imm & 0x800U)
        imm = static_cast<int64_t>(imm | 0xf000U);

    cpu.pc += (imm - 2);
}

static void beqz(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if (imm & 0x100)
        imm = static_cast<int64_t>(imm | 0xfe00U);

    if (!cpu.int_regs[rs1])
        cpu.pc += (imm - 2);
}

static void bnez(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if (imm & 0x100)
        imm = static_cast<int64_t>(imm | 0xfe00U);

    if (cpu.int_regs[rs1])
        cpu.pc += (imm - 2);
}

static void slli(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t shamt = decoder.shamt_c();

    cpu.int_regs[rd] <<= shamt;
}

static void fldsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = mmu.load(cpu.int_regs[IRegs::sp] + off, 64);

    cpu.flt_regs[rd] = reinterpret_cast<double>(val);
}

static void lwsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0xc0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x1cU);

    uint32_t val = mmu.load(cpu.int_regs[IRegs::sp] + off, 32);

    cpu.int_regs[rd] = static_cast<int64_t>(val);
}

static void ldsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0x1c0U) | ((decoder.insn >> 7U) & 0x20U) |
                      ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = mmu.load(cpu.int_regs[IRegs::sp] + off, 64);

    cpu.int_regs[rd] = val;
}

static void op4(Decoder decoder)
{
	uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
	uint32_t val2 = (decoder.insn >> 2U) & 0x1fU;

	uint64_t rd = decoder.rd();
	uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

	if (val1) {
		if (val2)
			cpu.int_regs[rd] = cpu.int_regs[rs1];
		else
			cpu.pc = (cpu.int_regs[rd] - 2);
	} else {
		if (val2)
			cpu.int_regs[rd] += cpu.int_regs[rs1];
		else {
			cpu.pc = (cpu.int_regs[rd] - 2);
			cpu.int_regs[IRegs::ra] = (cpu.pc + 2);
		}
	}
}

static void fsdsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0x1c0U) | 
					((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[IRegs::sp] + off;
    uint64_t rs1_bits = reinterpret_cast<uint64_t>(cpu.flt_regs[rs1]);

    mmu.store(addr, rs1_bits, 64);
}

static void swsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0xc0U) | 
					((decoder.insn >> 7U) & 0x3cU);

    uint64_t addr = cpu.int_regs[IRegs::sp] + off;
    uint32_t val = cpu.int_regs[rs1];

    mmu.store(addr, val, 32);
}

static void sdsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0x1c0U) | 
					((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu.int_regs[IRegs::sp] + off;
    uint64_t val = cpu.int_regs[rs1];

    mmu.store(addr, val, 64);
}

}; // namespace C

namespace LD {

static void funct3(Decoder decoder)
{
	switch (static_cast<LdType>(decoder.funct3())) {
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
			decoder.insn
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

namespace B {

static void funct3(Decoder decoder)
{
	uint64_t val1 = cpu.int_regs[decoder.rs1()];
	uint64_t val2 = cpu.int_regs[decoder.rs2()];
	
	int64_t val1_s = static_cast<int64_t>(val1);
	int64_t val2_s = static_cast<int64_t>(val2);
	int64_t imm = decoder.imm_b() - 4;

	switch (static_cast<BType>(decoder.funct3())) {
	case BType::BEQ:
		if (val1_s == val2_s)
			cpu.pc += imm;
		break;
	case BType::BNE:
		if (val1_s != val2_s)
			cpu.pc += imm;
		break;
	case BType::BLT:
		if (val1_s < val2_s)
			cpu.pc += imm;
		break;
	case BType::BGE:
		if (val1_s >= val2_s)
			cpu.pc += imm;
		break;
	case BType::BLTU:
		if (val1 < val2)
			cpu.pc += imm;
		break;
	case BType::BGEU:
		if (val1 >= val2)
			cpu.pc += imm;
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace B

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

	uint64_t addr = cpu.int_regs[rs1];
	
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

namespace O {

static void fence(Decoder decoder)
{
	
}

static void jal(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_j();

	cpu.int_regs[rd] = cpu.pc + 4;
	cpu.pc += imm - 4;
}

static void jalr(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	int64_t imm = decoder.imm_i();

	uint64_t tmp = cpu.pc + 4;
	
	cpu.pc = ((cpu.int_regs[rs1] + imm) & ~1) - 4;
	cpu.int_regs[rd] = tmp;
}

static void auipc(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	int64_t imm = static_cast<int64_t>(
		decoder.insn & 0xfffff000U
	);

	cpu.int_regs[rd] = cpu.pc + imm;
}

static void lui(Decoder decoder)
{
	uint64_t rd = decoder.rd();

	cpu.int_regs[rd] = static_cast<int64_t>(
		decoder.insn & 0xfffff000U
	);
}

}; // namespace O

}; // namespace Instruction
