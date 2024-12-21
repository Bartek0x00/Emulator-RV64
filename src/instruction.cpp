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
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();
	
	uint64_t addr = cpu.int_regs[rs1] + imm;
	
	if (cpu.exc_val != Exception::NONE)
		return;

	switch (static_cast<LdType>(decoder.funct3())) {
	case LdType::LB:
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 8)
		);
		break;
	case LdType::LH:
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 16)
		);
		break;
	case LdType::LW:
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 32)
		);
		break;
	case LdType::LD:
		cpu.int_regs[rd] = static_cast<int64_t>(
			mmu.load(addr, 64)
		);
		break;
	case LdType::LBU:
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 8)
		);
		break;
	case LdType::LHU:
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 16)
		);
		break;
	case LdType::LWU:
		cpu.int_regs[rd] = static_cast<uint64_t>(
			mmu.load(addr, 32)
		);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace LD

namespace ST {

static void funct3(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	int64_t imm = decoder.imm_s();

	uint64_t addr = cpu.int_regs[rs1] + imm;
	uint64_t value = cpu.int_regs[rs2];

	switch (static_cast<StType>(decoder.funct3())) {
	case StType::SB:
		mmu.store(addr, value, 8);
		break;
	case StType::SH:
		mmu.store(addr, value, 16);
		break;
	case StType::SW:
		mmu.store(addr, value, 32);
		break;
	case StType::SD:
		mmu.store(addr, value, 64);
		break;
	}
}

}; // namespace ST

namespace R {

static void funct3(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	
	uint64_t uval1 = cpu.int_regs[rs1];
	uint64_t uval2 = cpu.int_regs[rs2];

	int64_t val1 = static_cast<int64_t>(uval1);
	int64_t val2 = static_cast<int64_t>(uval2);
	
	std::int128_t lval1 = val1;
	std::int128_t lval2 = val2;
	
	std::uint128_t ulval1 = uval1;
	std::uint128_t ulval2 = uval2;

	switch (static_cast<RType>(decoder.funct3())) {
	case RType::ADDMULSUB:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::ADD:
			cpu.int_regs[rd] = val1 + val2;
			break;
		case RType::MUL:
			cpu.int_regs[rd] = val1 * val2;
			break;
		case RType::SUB:
			cpu.int_regs[rd] = val1 - val2;
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case RType::SLLMULH:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::SLL:
			cpu.int_regs[rd] = val1 << val2;
			break;
		case RType::MULH:
			cpu.int_regs[rd] = (lval1 * lval2) >> 64;
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case RType::SLTMULHSU:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::SLT:
			cpu.int_regs[rd] = val1 < val2;
			break;
		case RType::MULHSU:
			ulval1 = static_cast<std::int128_t>(val1);
			cpu.int_regs[rd] = (ulval1 * ulval2) >> 64;
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case RType::SLTUMULHU:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::SLTU:
			cpu.int_regs[rd] = uval1 < uval2;
			break;
		case RType::MULHU:
			cpu.int_regs[rd] = (ulval1 * ulval2) >> 64;
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case RType::XORDIV:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::XOR:
			cpu.int_regs[rd] = uval1 ^ uval2;
			break;
		case RType::DIV:
			switch (val2) {
			case -1:
				if (val1 == std::numeric_limits<int32_t>::min())
					cpu.int_regs[rd] = val1;
				break;
			case 0:
				cpu.int_regs[rd] = ~0ULL;
				break;
			default:
				cpu.int_regs[rd] = val1 / val2;
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
		break;
	case RType::SR:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::SRL:
			cpu.int_regs[rd] = uval1 >> uval2;
			break;
		case RType::DIVU:
			if (val2)
				cpu.int_regs[rd] = uval1 / uval2;
			else
				cpu.int_regs[rd] = ~0ULL;
			break;
		case RType::SRA:
		{
			int32_t hval1 = uval1;
			cpu.int_regs[rd] = hval1 >> val2;
			break;
		}
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case RType::ORREM:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::OR:
			cpu.int_regs[rd] = uval1 | uval2;
			break;
		case RType::REM:
			switch (val2) {
			case -1:
				if (val1 == std::numeric_limits<int64_t>::min())
					cpu.int_regs[rd] = 0;
				break;
			case 0:
				cpu.int_regs[rd] = val1;
				break;
			default:
				cpu.int_regs[rd] = val1 % val2;
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
		break;
	case RType::ANDREMU:
		switch (static_cast<RType>(decoder.funct7())) {
		case RType::AND:
			cpu.int_regs[rd] = uval1 & uval2;
			break;
		case RType::REMU:
			if (val2)
				cpu.int_regs[rd] = uval1 % uval2;
			else
				cpu.int_regs[rd] = uval1;
			break;
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

}; // namespace R

namespace R64 {

static void funct3(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();

	switch (static_cast<R64Type>(decoder.funct3())) {
	case R64Type::ADDSUBW:
	{
		uint64_t val1 = cpu.int_regs[rs1];
		int64_t val2 = cpu.int_regs[rs2];

		switch (static_cast<R64Type>(decoder.funct7())) {
		case R64Type::ADDW:
			cpu.int_regs[rd] = static_cast<int64_t>(
				val1 + val2
			);
			break;
		case R64Type::MULW:
			cpu.int_regs[rd] = static_cast<int64_t>(
				val1 * val2
			);
			break;
		case R64Type::SUBW:
			cpu.int_regs[rd] = static_cast<int64_t>(
				val1 - val2
			);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case R64Type::DIVW:
	{
		int32_t val1 = cpu.int_regs[rs1];
		int32_t val2 = cpu.int_regs[rs2];

		switch (val2) {
		case -1:
			if (val1 == std::numeric_limits<int32_t>::min())
				cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case 0:
			cpu.int_regs[rd] = ~0ULL;
			break;
		default:
			cpu.int_regs[rd] = static_cast<int64_t>(val1 / val2);
			break;
		}
		break;
	}
	case R64Type::SLLW:
	{
		uint64_t val1 = cpu.int_regs[rs1];
		uint64_t val2 = cpu.int_regs[rs2] & 0x1f;

		cpu.int_regs[rd] = static_cast<int64_t>(
			val1 << val2
		);
		break;
	}
	case R64Type::SRW:
		switch (static_cast<R64Type>(decoder.funct7())) {
		case R64Type::SRLW:
		{
			uint32_t val1 = cpu.int_regs[rs1];
			uint64_t val2 = cpu.int_regs[rs2] & 0x1f;

			cpu.int_regs[rd] = static_cast<int64_t>(
				val1 >> val2
			);
			break;
		}
		case R64Type::DIVUW:
		{
			uint32_t val1 = cpu.int_regs[rs1];
			uint32_t val2 = cpu.int_regs[rs2];

			if (val2)
				cpu.int_regs[rd] = static_cast<int64_t>(
					val1 / val2
				);
			else
				cpu.int_regs[rd] = ~0ULL;

			break;
		}
		case R64Type::SRAW:
		{
			int32_t val1 = cpu.int_regs[rs1];
			uint64_t val2 = cpu.int_regs[rs2] & 0x1f;

			cpu.int_regs[rd] = static_cast<int64_t>(
				val1 >> val2
			);
			break;
		}
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case R64Type::REMW:
	{
		int64_t val1 = static_cast<int32_t>(
			cpu.int_regs[rs1]
		);
		int64_t val2 = cpu.int_regs[rs2];

		cpu.int_regs[rd] = static_cast<int64_t>(
			val1 % val2
		);
		break;
	}
	case R64Type::REMUW:
	{
		uint64_t val1 = cpu.int_regs[rs1];
		int64_t val2 = cpu.int_regs[rs2];

		cpu.int_regs[rd] = val1 % val2;
		break;
	}
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
		break;
	}
}

}; // namespace R64

namespace I {

static void funct3(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	int64_t imm = decoder.imm_i();
	uint64_t uimm = static_cast<uint64_t>(imm);
	uint32_t shamt = decoder.shamt();
	
	switch (static_cast<IType>(decoder.funct3()) {
	case IType::ADDI:
		cpu.int_regs[rd] = cpu.int_regs[rs1] + imm;
		break;
	case IType::SLLI:
		cpu.int_regs[rd] = cpu.int_regs[rs1] << shamt;
		break;
	case IType::SLTI:
		cpu.int_regs[rd] = static_cast<int64_t>(
			cpu.int_regs[rs1]
		) < imm;
		break;
	case IType::SLTIU:
		cpu.int_regs[rd] = cpu.int_regs[rs1] < uimm;
		break;
	case IType::XORI:
		cpu.int_regs[rd] = cpu.int_regs[rs1] ^ uimm;
		break;
	case IType::SRI:
		switch (static_cast<IType>(decoder.funct7() >> 1)) {
		case IType::SRLI:
			cpu.int_regs[rd] = cpu.int_regs[rs1] >> shamt;
			break;
		case IType::SRAI:
			cpu.int_regs[rd] = static_cast<int64_t>(
				cpu.int_regs[rs1]
			) >> uimm; 
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case IType::ORI:
		cpu.int_regs[rd] = cpu.int_regs[rs1] | uimm;
		break;
	case IType::ANDI:
		cpu.int_regs[rd] = cpu.int_regs[rs1] & uimm;
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace I

namespace I64 {

static void funct3(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();

	switch (static_cast<I64Type>(decoder.funct3())) {
	case I64Type::ADDIW:
	{
		int64_t imm = decoder.imm_i();
		cpu.int_regs[rd] = static_cast<int64_t>(
			cpu.int_regs[rs1] + imm
		);
		break;
	}
	case I64Type::SLLIW:
	{
		uint64_t shamt = decoder.shamt();
		cpu.int_regs[rd] = static_cast<int64_t>(
			cpu.int_regs[rs1] << shamt;
		);
		break;
	}
	case I64Type::SRIW:
	{
		uint32_t shamt = decoder.shamt();
		switch (static_cast<I64Type>(decoder.funct7())) {
		case I64Type::SRLIW:
			cpu.int_regs[rd] = static_cast<int64_t>(
				static_cast<uint32_t>(rs1) >> shamt
			);
			break;
		case I64Type::SRAIW:
			cpu.int_regs[rd] = static_cast<int64_t>(
				static_cast<int32_t>(rs1) >> shamt
			);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		}
		break;
	}
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace I64

namespace FD {

enum class FValue : uint64_t {
	INF = 1 << 0,
	NORMAL = 1 << 1,
	SUBNORMAL = 1 << 2,
	NEG_ZERO = 1 << 3,
	POS_ZERO = 1 << 4,
	POS_SUBNORMAL = 1 << 5,
	POS_NORMAL = 1 << 6,
	POS_INF = 1 << 7,
	NAN_SIG = 1 << 8,
	NAN_QUIET = 1 << 9
};

static void set_exceptions(void)
{
	int exceptions = std::fetestexcept(FE_ALL_EXCEPT);
	
	CRegs::FExcept current = CRegs::FExcept::INVALID;

	if (exceptions & FE_DIVBYZERO)
		current = CRegs::FExcept::DIVBYZERO;
	
	if (exceptions & FE_INEXACT)
		current = CRegs::FExcept::INEXACT
	
	if (exceptions & FE_OVERFLOW)
		current = CRegs::FExcept::OVERFLOW;

	if (exceptions & FE_UNDERFLOW)
		current = CRegs::FExcept::UNDERFLOW;
	
	if (current != CRegs::FExcept::INVALID)
		cpu.csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu.csr_regs.load(CRegs::Address::FFLAGS) |
				current
		);
	
	if (exceptions)
		std::feclearexcept(FE_ALL_EXCEPT);
}

static inline bool is_fs(void)
{
	uint64_t fs = read_bits(
		cpu.csr_regs.load(
			CRegs::Address::MSTATUS
		),
		14, 13
	);

	if (static_cast<CRegs::FS>(fs) != FS::OFF)
		return true;
	
	cpu.set_exception(
		Exception::ILLEGAL_INSTRUCTION
	);
	return false;
}

static void fl(Decoder decoder)
{
	if (!is_fs())
		return;
	
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t off = decoder.fl_off();

	uint64_t addr = cpu.int_regs[rs1] + off;
	uint64_t val;

	switch (static_cast<FdType>(decoder.funct3())) {
	case FdType::FLW:
		val = mmu.load(addr, 32) | 0xffffffff00000000;
		cpu.flt_regs[rd] = reinterpret_cast<double>(val);
		break;
	case FdType::FLD:
		val = mmu.load(addr, 64);
		cpu.flt_regs[rd] = reinterpret_cast<double>(val);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

static void fs(Decoder decoder)
{
	if (!is_fs())
		return;
	
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t off = decoder.fs_off();

	uint64_t addr = cpu.int_regs[rs1] + off;
	uint64_t rs2f_bits = cpu.flt_regs[rs2];

	switch (static_cast<FdType>(decoder.funct3())) {
	case FdType::FSW:
		mmu.store(addr, rs2f_bits, 32);
		break;
	case FdType::FSD:
		mmu.store(addr, rs2f_bits, 64);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

static void fmadd(Decoder decoder)
{
	if (!is_fs())
		return;
	
	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FMADDS:
		fmadds
		break;
	case FdType::FMADDD:
		fmaddd
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

static void fmsub(Decoder decoder)
{
	if (!is_fs())
		return;

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FMSUBS:
		fmsubs
		break;
	case FdType::FMSUBD:
		fmsubd
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION.
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

static void fnmadd(Decoder decoder)
{
	if (!is_fs())
		return;
	
	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FNMADDS:
		fnmadds
		break;
	case FdType::FNMADDD:
		fnmaddd
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

static void fnmsub(Decoder decoder)
{
	if (!is_fs())
		return;

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FNMSUBS:
		fnmsubs
		break;
	case FdType::FNMSUBD:
		fnmsubd
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

static void fother(Decoder decoder)
{
	if (!is_fs())
		return;
	
	switch (static_cast<FdType>(decoder.funct7()) {
	case FdType::FADDS:
		fadds
		break;
	case FdType::FADDD:
		faddd
		break;
	case FdType::FSUBS:
		fsubs
		break;
	case FdType::FSUBD:
		fsubd
		break;
	case FdType::FMULS:
		fmuls
		break;
	case FdType::FMULD:
		fmuld
		break;
	case FdType::FDIVS:
		fdivs
		break;
	case FdType::FDIVD:
		fdivd
		break;
	case FdType::FSNGJS:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FSGNJ:
			fsgnjs
			break;
		case FdType::FSGNJN:
			fsgnjns
			break;
		case FdType::FSGNJX:
			fsgnjxs
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn			
			);
			break;
		}
		break;
	case FdType::FSNGJD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FSGNJ:
			fsgnjd
			break;
		case FdType::FSGNJN:
			fsgnjnd
			break;
		case FdType::FSGNJX:
			fsgnjxd
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FMINMAXS:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::MIN:
			fmins
			break;
		case FdType::MAX:
			fmaxs
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FMINMAXD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::MIN:
			fmind
			break;
		case FdType::MAX:
			fmaxd
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		}
		break;
	case FdType::FCVTSD:
		fcvtsd
		break;
	case FdType::FCVTDS:
		fcvtds
		break;
	case FdType::FSQRTS:
		fsqrts
		break;
	case FdType::FSQRTD:
		fsqrtd
		break;
	case FdType::FCS:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FLE:
			fles
			break;
		case FdType::FLT:
			flts
			break;
		case FdType::FEQ:
			feqs
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FCD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FLE:
			fled
			break;
		case FdType::FLT:
			fltd
			break;
		case FdType::FEQ:
			feqd
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FCVTS:
		fcvtss
		break;
	case FdType::FCVTD:
		fcvtsd
		break;
	case FdType::FCVTSW:
		fcvtsw
		break;
	case FdType::FCVTDW:
		fcvtdw
		break;
	case FdType::FMVXW:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FMV:
			fmvs
			break;
		case FdType::FCLASS:
			fclasss
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FMVXD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FMV:
			fmvd
			break;
		case FdType::FCLASS:
			fclassd
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case FdType::FMVWX:
		fmvsx
		break;
	case FdType::FMVDX:
		fmvdx
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

}; // namespace FD

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
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t addr = cpu.int_regs[rs1];

	switch (static_cast<AType>(decoder.funct3())) {
	case AType::AMOW:
	{
		if (cpu.int_regs[rs1] % 4)
			cpu.set_exception(
				Exception::INSTRUCTION_ADDRESS_MISALIGNED,
				decoder.insn
			);

		int32_t val1 = mmu.load(addr, 32);
		int32_t val2 = cpu.int_regs[rs2];

		switch (static_cast<AType>(decoder.funct5())) {
		case AType::ADD:
			mmu.store(addr, val1 + val2, 32);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::SWAP:
			mmu.store(addr, val2, 32);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::LR:
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			cpu.reservations.insert(addr);
			break;
		case AType::SC:
			if (cpu.reservations.contains(addr)) {
				cpu.reservations.erase(addr);
				mmu.store(
					addr, 
					static_cast<uint32_t>(val2), 
					32
				);
				cpu.int_regs[rd] = 0;
			} else {
				cpu.int_regs[rd] = 1;
			}
			break;
		case AType::XOR:
			mmu.store(
				addr, 
				static_cast<int64_t>(val1 ^ val2), 
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::OR:
			mmu.store(
				addr,
				static_cast<int64_t>(val1 | val2),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::AND:
			mmu.store(
				addr,
				static_cast<int64_t>(val1 & val2),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MIN:
			mmu.store(
				addr,
				static_cast<int64_t>(std::min(val1, val2)),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MAX:
			mmu.store(
				addr,
				static_cast<int64_t>(std::max(val1, val2)),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MINU:
			mmu.store(
				addr,
				std::min(
					static_cast<uint32_t>(val1),
					static_cast<uint32_t>(val2)
				),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MAXU:
			mmu.store(
				addr,
				std::max(
					static_cast<uint32_t>(val1),
					static_cast<uint32_t>(val2)
				),
				32
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case AType::AMOD:
		
		if (cpu.int_regs[rs1] % 8)
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		
		int64_t val1 = mmu.load(addr, 64);
		int64_t val2 = cpu.int_regs[rs2];

		switch (static_cast<AType>(decoder.funct5())) {
		case AType::ADD:
			mmu.store(
				addr, 
				static_cast<int64_t>(val1 + val2),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::SWAP:
			mmu.store(addr, val2, 64);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::LR:
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			cpu.reservations.insert(addr);
			break;
		case AType::SC:
			if (cpu.reservations.contains(addr)) {
				cpu.reservations.erase(addr);
				mmu.store(
					addr,
					static_cast<uint64_t>(val2),
					64
				);
				cpu.int_regs[rd] = 0;
			} else {
				cpu.int_regs[rd] = 1;
			}
			break;
		case AType::XOR:
			mmu.store(
				addr,
				static_cast<int64_t>(val1 ^ val2),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::OR:
			mmu.store(
				addr,
				static_cast<int64_t>(val1 | val2),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::AND:
			mmu.store(
				addr,
				static_cast<int64_t>(val1 & val2),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MIN:
			mmu.store(
				addr,
				static_cast<int64_t>(std::min(val1, val2)),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MAX:
			mmu.store(
				addr,
				static_cast<int64_t>(std::max(val1, val2)),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MINU:
			mmu.store(
				addr,
				std::min(
					static_cast<uint64_t>(val1),
					static_cast<uint64_t>(val2)
				),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case AType::MAXU:
			mmu.store(
				addr,
				std::max(
					static_cast<uint64_t>(val1),
					static_cast<uint64_t>(val2)
				),
				64
			);
			cpu.int_regs[rd] = static_cast<int64_t>(val1);
			break;
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

}; // namespace A

namespace CSR {

using op_t = std::function<
	uint64_t(uint64_t, uint64_t)
>;

using handler_t = std::function<
	void(Decoder, uint64_t, uint64_t, op_t)
>;

static void default_h(Decoder decoder, 
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);

	cpu.csr_regs.store(csr, op(csr_val, rhs));
	cpu.int_regs[rd] = csr_val;
}

static void readonly_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);

	cpu.int_regs[rd] = csr_val;
}

static void priviledged_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	if (cpu.mode == Cpu::Mode::USER) {
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);

	cpu.csr_regs.store(csr, op(csr_val, rhs));
	cpu.int_regs[rd] = csr_val;
}

static void enforced_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);
	uint64_t opval = op(csr_val, rhs);

	if (csr_val != opval) {
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	cpu.int_regs[rd] = csr_val;
}

static void fcsr_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);
	uint64_t fexceptions = cpu.csr_regs.load(
		CRegs::Address::FFLAGS
	) & CRegs::FExcept::MASK;

	uint64_t op_val = op(csr_val | fexceptions, rhs);

	cpu.csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu.csr_regs.load(CRegs::Address::FFLAGS) & 
			~CRegs::FExcept::MASK
	);

	cpu.csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu.csr_regs.load(CRegs::Address::FFLAGS) | 
			op_val & CRegs::FExcept::MASK
	);

	cpu.csr_regs.store(
		CRegs::Address::FCSR, 
		op_val & 0xff
	);
	cpu.int_regs[rd] = csr_val & 0xff;
}

static void fflags_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);
	uint64_t fexceptions = csr_val & CRegs::FExcept::MASK;
	uint64_t op_val = op(fexceptions, rhs);

	cpu.csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu.csr_regs.load(CRegs::Address::FFLAGS) &
			~CRegs::FExcept::MASK
	);

	cpu.csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu.csr_regs.load(CRegs::Address::FFLAGS) |
			op_val & CRegs::FExcept::MASK
	);
	
	uint64_t fcsr = cpu.csr_regs.load(CRegs::Address::FCSR);
	fcsr &= ~CRegs::FExcept::MASK;
	fcsr |= op_val & 0xff;
	
	cpu.csr_regs.store(CRegs::Address::FCSR, fcsr);
	cpu.int_regs[rd] = csr_val & CRegs::FExcept::MASK;
}

static void frm_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(CRegs::Address::FCSR);
	csr_val >>= 5;
	uint64_t op_val = op(csr_val, rhs);

	cpu.csr_regs.store(
		CRegs::Address::FRM,
		~Cpu::FPURoundingMode::MASK
	);

	cpu.csr_regs.store(
		CRegs::Address::FRM,
		cpu.csr_regs.load(CRegs::Address::FRM) |
			op_val & Cpu::FPURoundingMode::MASK
	);

	uint64_t fcsr = cpu.csr_regs.load(CRegs::Address::FCSR);
	fcsr &= ~0xe0;
	fcsr |= op_val << 5;
	fcsr &= 0xff;

	cpu.csr_regs.store(CRegs::Address::FCSR, fcsr);
	cpu.int_regs[rd] = csr_val & Cpu::FPURoundingMode::MASK;
}

static void satp_h(Decoder decoder
	uint64_t csr, uint64_t rhs, op_t op)
{
	if (cpu.csr_regs.read_bit_mstatus(CRegs::Mask::TVM))
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
	
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu.csr_regs.load(csr);

	cpu.csr_regs.store(csr, op(csr_val, rhs));
	cpu.int_regs[rd] = csr_val;

	mmu.update();
}

static std::array<handler_t, 4096> csr_handlers = [](void) {
	std::array<handler_t, 4096> tmp{default_h};
	
	tmp[CRegs::Address::FCSR] = fcsr_h;
	tmp[CRegs::Address::FFLAGS] = fflags_h;
	tmp[CRegs::Address::FRM] = frm_h;
	tmp[CRegs::Address::SATP] = satp_h;

	tmp[CRegs::Address::MVENDORID] = readonly_h;
	tmp[CRegs::Address::MARCHID] = readonly_h;
	tmp[CRegs::Address::MIMPID] = readonly_h;
	tmp[CRegs::Address::MHARTID] = readonly_h;
	tmp[CRegs::Address::MISA] = readonly_h;
	tmp[CRegs::Address::TDATA1] = readonly_h;

	tmp[CRegs::Address::CYCLE] = enforced_h;
	tmp[CRegs::Address::MSTATUS] = priviledged_h;

	return tmp;
}();

static void sret(Decoder decoder)
{
	uint64_t tsr = read_bit(
		cpu.csr_regs.load(CRegs::Address::MSTATUS),
		CRegs::Mask::TSR
	);
	
	if (tsr || cpu.mode == Cpu::Mode::USER) {
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	cpu.pc = cpu.csr_regs.load(CRegs::Address::SEPC) - 4;
	cpu.mode = static_cast<Cpu::Mode>(
		read_bit(
			cpu.csr_regs.load(CRegs::Address::SSTATUS),
			CRegs::Mask::SPP
		)
	);

	if (cpu.mode == Cpu::Mode::USER)
		cpu.csr_regs.store(
			CRegs::Address::MSTATUS,
			write_bit(
				cpu.csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mask::MPRV, 
				0
			)
		);

	cpu.csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu.csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Mask::SIE,
			read_bit(
				cpu.csr_regs.load(
					CRegs::Address::SSTATUS
				),
				CRegs::Mask::SPIE
			)
		)
	);

	cpu.csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu.csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Mask::SPIE, 
			1
		)
	);

	cpu.csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu.csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Mask::SPP, 
			Cpu::Mode::USER
		)
	);
}

static void mret(Decoder decoder)
{
	if (cpu.mode != Cpu::Mode::MACHINE) {
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	cpu.pc = cpu.csr_regs.load(CRegs::Address::MEPC) - 4;
	
	Cpu::Mode mpp = read_bits(
		cpu.csr_regs.load(
			CRegs::Address::MSTATUS
		), 
		12, 11
	);

	switch (mpp) {
	case Cpu::Mode::USER:
	case Cpu::Mode::SUPERVISOR:
	case Cpu::Mode::MACHINE:
		cpu.mode = mpp;
	default:
		cpu.mode = Cpu::Mode::INVALID;
	}

	if (cpu.mode != Cpu::Mode::MACHINE)
		cpu.csr_regs.store(
			CRegs::Address::MSTATUS,
			write_bit(
				cpu.csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mask::MPRV,
				0
			)	
		);
	
	cpu.csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bit(
			cpu.csr_regs.load(
				CRegs::Address::MSTATUS
			),
			CRegs::Mask::MIE,
			read_bit(
				cpu.csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mask::MPIE
			)
		)
	);

	cpu.csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bit(
			CRegs::Address::MSTATUS,
			CRegs::Mask::MPIE,
			1
		)
	);

	cpu.csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bits(
			cpu.csr_regs.load(
				CRegs::Address::MSTATUS
			),
			12, 11,
			Cpu::Mode::USER
		)
	);
}

static void environment(Decoder decoder)
{
	uint64_t imm = decoder.rs2();
	uint64_t funct7 = decoder.funct7();

	switch (static_cast<CSRType>(funct7)) {
	case CSRType::SFENCEVMA7:
		if (read_bit(
				cpu.csr_regs.load(CRegs::Address::MSTATUS), 
				CRegs::Mask::TVM
			) || cpu.mode == Cpu::Mode::USER)
		{	
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		}
		mmu.update();
		break;
	case CSRType::HFENCEBVMA7:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	case CSRType::HFENCEGVMA7:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	switch (static_cast<CSRType>(imm) {
	case CSRType::ECALL:
		switch (static_cast<Cpu::Mode>(cpu.mode) {
		case Cpu::Mode::MACHINE:
			cpu.set_exception(
				Exception::ECallMMode,
				cpu.pc
			);
			break;
		case Cpu::Mode::SUPERVISOR:
			cpu.set_exception(
				Exception::ECallSMode,
				cpu.pc
			);
			break;
		case Cpu::Mode::USER:
			cpu.set_exception(
				Exception::ECallUMode,
				cpu.pc
			);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case CSRType::EBREAK:
		cpu.set_exception(
			Exception::BREAKPOINT
		);
		break;
	case CSRType::RET:
		switch (static_cast<CSRType>(funct7)) {
		case CSRType::URET7:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		case CSRType::SRET7:
			sret(decoder);
			break;
		case CSRType::MRET7:
			mret(decoder);
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case CSRType::WFI:
		switch (static_cast<CSRType>(funct7)) {
		case CSRType::WFI7:
			break;
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

static void funct3(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t csr = decoder.csr();
	uint64_t rhs = cpu.int_regs[rs1];

	switch (static_cast<CSRType>(decoder.funct3())) {
	case CSRType::ENVIRONMENT:
		environment(decoder);
		break;
	case CSRType::CSRW:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return rhs;
			}
		);
		break;
	case CSRType::CSRS:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr | rhs;
			}
		);
		break;
	case CSRType::CSRC:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr & ~rhs;
			}
		);
		break;
	case CSRType::CSRWI:
		rhs = rs1;

		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return rhs;
			}
		);
		break;
	case CSRType::CSRSI:
		rhs = rs1;

		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr | rhs;
			}
		);
		break;
	case CSRType::CSRCI:
		rhs = rs1;

		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr & ~rhs;
			}
		);
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace CSR

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
