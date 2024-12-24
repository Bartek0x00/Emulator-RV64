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

static inline float get_cnanf(void)
{
	uint32_t nan_raw = 0x7fc00000U;
	return bit_cast<float>(nan_raw);
}

static inline double get_cnand(void)
{
	uint64_t nan_raw = 0x7ff8000000000000ULL;
	return bit_cast<double>(nan_raw);
}

static inline bool is_cnanf(float valf)
{
	uint32_t nan_raw = 0x7fc00000U;
	uint32_t val = bit_cast<uint32_t>(valf);
	return nan_raw == val;
}

static inline bool is_cnand(double vald)
{
	uint64_t nan_raw = 0x7ff8000000000000ULL;
	uint64_t val = bit_cast<uint64_t>(vald);
	return nan_raw == val;
}

static inline bool is_snanf(float valf)
{
	uint32_t snan_raw = 0x7f800001U;
	uint32_t val = bit_cast<uint32_t>(valf);
	return snan_raw == val;
}

static inline bool is_snand(double vald)
{
	uint64_t snan_raw = 0x7ff0000000000001ULL;
	uint64_t val = bit_cast<uint64_t>(vald);
	return snan_raw == val;
}

template<typename T>
static T round(T val, Decoder decoder)
{
	T newval = val;
	uint64_t rm = decoder.rounding_mode();

	switch (static_cast<Cpu::FPURoundingMode>(rm)) {
	case FPURoundingMode::ROUND_TO_NEAREST:
	case FPURoundingMode::ROUND_NEAREST_MAX_MAGNITUDE:
		newval = std::round(val);
		break;
	case FPURoundingMode::ROUND_TO_ZERO:
		newval = std::trunc(val);
		break;
	case FPURoundingMode::ROUND_DOWN:
		newval = std::floor(val);
		break;
	case FPURoundingMode::ROUND_UP:
		newval = std::ceil(val);
		break;
	case FPURoundingMode::ROUND_DYNAMIC:
		break;
	default:
		cpu.set_exception(
			Exception::ILLEGAL_INSTRUCTION
		);
	}

	if (newval != val)
		cpu.csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu.csr_regs.load(
				CRegs::Address::FFLAGS
			) | CRegs::FExcept::INEXACT
		);
	
	return newval;
}

template<typename T1, typename T2>
static T1 try_convert(T2 val)
{
	constexpr T1 min = std::numeric_limits<T1>::min();
	constexpr T1 max = std::numeric_limits<T1>::max();

	bool is_nan = std::isnan(val);
	
	if (is_nan || val < min || val >= max)
		cpu.csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu.csr_regs.load(
				CRegs::Address::FFLAGS
			) | CRegs::FExcept::INVALID
		);

	if (is_nan)
		val = max;
	
	return val;
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
	
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rs3 = decoder.rs3();

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FMADDS:
	{
		float val1 = cpu.flt_regs[rs1];
		float val2 = cpu.flt_regs[rs2];
		float val3 = cpu.flt_regs[rs3];

		float res = (val1 * val2) + val3;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
		
		cpu.flt_regs[rd] = res;
		break;
	}
	case FdType::FMADDD:
	{
		double val1 = cpu.flt_regs[rs1];
		double val2 = cpu.flt_regs[rs2];
		double val3 = cpu.flt_regs[rs3];

		double res = (val1 * val2) + val3;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
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

	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rs3 = decoder.rs3();

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FMSUBS:
	{
		float val1 = cpu.flt_regs[rs1];
		float val2 = cpu.flt_regs[rs2];
		float val3 = cpu.flt_regs[rs3];

		float res = (val1 * val2) - val3;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
	case FdType::FMSUBD:
	{
		double val1 = cpu.flt_regs[rs1];
		double val2 = cpu.flt_regs[rs2];
		double val3 = cpu.flt_regs[rs3];

		double res = (val1 * val2) - val3;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
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
	
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rs3 = decoder.rs3();

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FNMADDS:
	{
		float val1 = -static_cast<float>(
			cpu.flt_regs[rs1]
		);
		float val2 = cpu.flt_regs[rs2];
		float val3 = cpu.flt_regs[rs3];

		float res = (val1 * val2) + val3;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
	case FdType::FNMADDD:
	{
		double val1 = -static_cast<double>(
			cpu.flt_regs[rs1]
		);
		double val2 = cpu.flt_regs[rs2];
		double val3	= cpu.flt_regs[rs3];

		double res = (val1 * val2) + val3;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
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

	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rs3 = decoder.rs3();

	switch (static_cast<FdType>(decoder.funct2())) {
	case FdType::FNMSUBS:
	{
		float val1 = -static_cast<float>(
			cpu.flt_regs[rs1]
		);
		float val2 = cpu.flt_regs[rs2];
		float val3 = cpu.flt_regs[rs3];

		float res = (val1 * val2) - val3;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
	case FdType::FNMSUBD:
	{
		double val1 = -static_cast<double>(
			cpu.flt_regs[rs1]
		);
		double val2 = cpu.flt_regs[rs2];
		double val3 = cpu.flt_regs[rs3];

		double res = (val1 * val2) - val3;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	}
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
	
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();

	float val1f = cpu.flt_regs[rs1];
	float val2f = cpu.flt_regs[rs2];

	double val1d = cpu.flt_regs[rs1];
	double val2d = cpu.flt_regs[rs2];

	switch (static_cast<FdType>(decoder.funct7()) {
	case FdType::FADDS:
		float res = val1f + val2f;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FADDD:
		double res = val1d + val2d;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FSUBS:
		float res = val1f - val2f;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FSUBD:
		double res = val1d - val2d;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
		
		cpu.flt_regs[rd] = res;
		break;
	case FdType::FMULS:
		float res = val1f * val2f;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FMULD:
		double res = val1d * val2d;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FDIVS:
		float res = val1f / val2f;
		if (is_cnanf(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FDIVD:
		double res = val1d / val2d;
		if (is_cnand(res))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = res;
		break;
	case FdType::FSNGJS:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FSGNJ:
			cpu.flt_regs[rd] = static_cast<float>(
				std::copysign(val1f, val2f)
			);
			break;
		case FdType::FSGNJN:
			val2f = -static_cast<float>(
				cpu.flt_regs[rs2]
			);
			cpu.flt_regs[rd] = static_cast<float>(
				std::copysign(val1f, val2f)
			);
			break;
		case FdType::FSGNJX:
		{
			uint32_t uval1 = cpu.flt_regs[rs1];
			uint32_t uval2 = cpu.flt_regs[rs2];

			uint32_t mask = 1ULL << 31;
			uint32_t res = uval1 & (mask - 1);
			res |= (uval1 & mask) ^ (uval2 & mask);
			
			cpu.flt_regs[rd] = reinterpret_cast<double>(res);
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
	case FdType::FSNGJD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FSGNJ:
			cpu.flt_regs[rd] = static_cast<double>(
				std::copysign(val1d, val2d)
			);
			break;
		case FdType::FSGNJN:
			val2d = -static_cast<double>(
				cpu.flt_regs[rs2]
			);
			cpu.flt_regs[rd] = static_cast<double>(
				std::copysign(val1d, val2d)
			);
			break;
		case FdType::FSGNJX:
		{
			uint64_t uval1 = cpu.flt_regs[rs1];
			uint64_t uval2 = cpu.flt_regs[rs2];

			uint64_t mask = 1ULL << 63;
			uint64_t res = uval1 & (mask - 1);
			res |= (uval1 & mask) ^ (uval2 & mask);

			cpu.flt_regs[rd] = reinterpret_cast<double>(res);
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
	case FdType::FMINMAXS:
	{
		bool is_val1_nan = std::isnan(val1f);
		bool is_val2_nan = std::isnan(val2f);
		
		if (is_snanf(val1f) || is_snanf(val2f))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		if (is_val1_nan && is_val2_nan) {
			cpu.flt_regs[rd] = get_cnanf();
			break;
		}

		if (is_val1_nan) {
			cpu.flt_regs[rd] = val2f;
			break;
		}

		if (is_val2_nan) {
			cpu.flt_regs[rd] = val1f;
			break;
		}

		if (val1f == val2f) {
			if (std::signbit(val1f)
				cpu.flt_regs[rd] = val1f;
			else
				cpu.flt_regs[rd] = val2f;
			break;
		}

		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::MIN:
			if (val1f < val2f)
				cpu.flt_regs[rd] = val1f;
			else
				cpu.flt_regs[rd] = val2f;
			break;
		case FdType::MAX:
			if (val1f > val2f)
				cpu.flt_regs[rd] = val1f;
			else
				cpu.flt_regs[rd] = val2f;
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
	case FdType::FMINMAXD:
		bool is_val1_nan = std::isnan(val1d);
		bool is_val2_nan = std::isnan(val2d);
		
		if (is_snand(val1d) || is_snand(val2d))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		if (is_val1_nan && is_val2_nan) {
			cpu.flt_regs[rd] = get_cnand();
			break;
		}

		if (is_val1_nan) {
			cpu.flt_regs[rd] = val2d;
			break;
		}

		if (is_val2_nan) {
			cpu.flt_regs[rd] = val1d;
			break;
		}

		if (val1d == val2d) {
			if (std::signbit(val1d)
				cpu.flt_regs[rd] = val1d;
			else
				cpu.flt_regs[rd] = val2d;
			break;
		}
		
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::MIN:
			if (val1d < val2d)
				cpu.flt_regs[rd] = val1d;
			else 
				cpu.flt_regs[rd] = val2d;
			break;
		case FdType::MAX:
			if (val1d > val2d)
				cpu.flt_regs[rd] = val1d;
			else
				cpu.flt_regs[rd] = val2d;
			break;
		default:
			cpu.set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		}
		break;
	case FdType::FCVTSD:
	{
		float val = static_cast<double>(
			cpu.flt_regs[rs1]
		);
		if (std::isnan(val))
			val = std::numeric_limits<float>::quiet_NaN();
		
		cpu.flt_regs[rd] = val;
		break;
	}
	case FdType::FCVTDS:
	{
		double val = static_cast<float>(
			cpu.flt_regs[rs1]
		);
		if (std::isnan(val))
			val = std::numeric_limits<double>::quiet_NaN();
		
		cpu.flt_regs[rd] = val;
		break;
	}
	case FdType::FSQRTS:
		if (val1f < static_cast<float>(0))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = static_cast<float>(
			std::sqrt(val1f)
		);
		break;
	case FdType::FSQRTD:
		if (val1d < static_cast<double>(0))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu.flt_regs[rd] = static_cast<double>(
			std::sqrt(val1d)
		);
		break;
	case FdType::FCS:
		if (std::isnan(val1f) || std::isnan(val2f))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FLE:
			cpu.int_regs[rd] = static_cast<bool>(
				val1f <= val2f
			);
			break;
		case FdType::FLT:
			cpu.int_regs[rd] = static_cast<bool>(
				val1f < val2f
			);
			break;
		case FdType::FEQ:
			cpu.int_regs[rd] = static_cast<bool>(
				val1f == val2f
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
	case FdType::FCD:
		if (std::isnan(val1d) || std::isnan(val2d))
			cpu.csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu.csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FLE:
			cpu.int_regs[rd] = static_cast<bool>(
				val1d <= val2d
			);
			break;
		case FdType::FLT:
			cpu.int_regs[rd] = static_cast<bool>(
				val1d < val2d
			);
			break;
		case FdType::FEQ:
			cpu.int_regs[rd] = static_cast<bool>(
				val1d == val2d
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
	case FdType::FCVTS:
	{
		float val = round(
			static_cast<float>(cpu.flt_regs[rs1]),
			decoder
		);

		switch (static_cast<FdType>(rs2)) {
		case FdType::FCVT0:
			cpu.int_regs[rd] = static_cast<int64_t>(
				try_convert<int32_t>(val)
			);
			break;
		case FdType::FCVT1:
			cpu.int_regs[rd] = static_cast<int64_t>(
				try_convert<uint32_t>(val)
			);
			break;
		case FdType::FCVT2:
			cpu.int_regs[rd] = try_convert<int64_t>(val);
			break;
		case FdType::FCVT3:
			cpu.int_regs[rd] = try_convert<uint64_t>(val);
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
	case FdType::FCVTD:
	{
		double val = round(
			static_cast<double>(cpu.flt_regs[rs1]),
			decoder
		);

		switch (static_cast<FdType>(rs2)) {
		case FdType::FCVT0:
			cpu.int_regs[rd] = static_cast<int64_t>(
				try_convert<int32_t>(val)
			);
			break;
		case FdType::FCVT1:
			cpu.int_regs[rd] = static_cast<int64_t>(
				try_convert<uint32_t>(val)
			);
			break;
		case FdType::FCVT2:
			cpu.int_regs[rd] = try_convert<int64_t>(val);
			break;
		case FdType::FCVT3:
			cpu.int_regs[rd] = try_convert<uint64_t>(val);
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
	case FdType::FCVTSW:
	{
		int64_t val = cpu.int_regs[rs1];
			
		switch (static_cast<FdType>(rs2)) {
		case FdType::FCVT0:
			cpu.flt_regs[rd] = static_cast<float>(
				static_cast<int32_t>(val)
			);
			break;
		case FdType::FCVT1:
			cpu.flt_regs[rd] = static_cast<float>(
				static_cast<uint32_t>(val)
			);
			break;
		case FdType::FCVT2:
			cpu.flt_regs[rd] = static_cast<float>(
				val
			);
			break;
		case FdType::FCVT3:
			cpu.flt_regs[rd] = static_cast<float>(
				static_cast<uint64_t>(val)
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
	case FdType::FCVTDW:
	{
		int64_t val = cpu.int_regs[rs1];

		switch (static_cast<FdType>(rs2)) {
		case FdType::FCVT0:
			cpu.flt_regs[rd] = static_cast<double>(
				static_cast<int32_t>(val)
			);
			break;
		case FdType::FCVT1:
			cpu.flt_regs[rd] = static_cast<double>(
				static_cast<uint32_t>(val)
			);
			break;
		case FdType::FCVT2:
			cpu.flt_regs[rd] = static_cast<double>(
				val
			);
			break;
		case FdType::FCVT3:
			cpu.flt_regs[rd] = static_cast<double>(
				static_cast<uint64_t>(val)
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
	case FdType::FMVXW:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FMV:
		{
			uint32_t val = reinterpret_cast<uint32_t>(
				static_cast<float>(
					cpu.flt_regs[rs1]
				)
			);
			cpu.int_regs[rd] = static_cast<int64_t>(
				val
			);
			break;
		}
		case FdType::FCLASS:
			float val = cpu.flt_regs[rs1];
			bool is_neg = std::signbit(val);

			switch (std::fpclassify(val)) {
			case FP_INFINITE:
				if (is_neg)
					cpu.int_regs[rd] = FValue::INF;
				else
					cpu.int_regs[rd] = FValue::POS_INF;
				break;
			case FP_NORMAL:
				if (is_neg)
					cpu.int_regs[rd] = FValue::NORMAL;
				else
					cpu.int_regs[rd] = FValue::POS_NORMAL;
				break;
			case FP_SUBNORMAL:
				if (is_neg)
					cpu.int_regs[rd] = FValue::SUBNORMAL;
				else
					cpu.int_regs[rd] = FValue::POS_SUBNORMAL;
				break;
			case FP_ZERO:
				if (is_neg)
					cpu.int_regs[rd] = FValue::NEG_ZERO;
				else
					cpu.int_regs[rd] = FValue::POS_ZERO;
				break;
			case FP_NAN:
				if (is_snanf(val))
					cpu.int_regs[rd] = FValue::NAN_SIG;
				else
					cpu.int_regs[rd] = FValue::NAN_QUIET;
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
	case FdType::FMVXD:
		switch (static_cast<FdType>(decoder.funct3())) {
		case FdType::FMV:
			uint64_t val = reinterpret_cast<uint64_t>(
				static_cast<double>(
					cpu.flt_regs[rs1]
				)
			);
			cpu.int_regs[rd] = static_cast<int64_t>(
				val
			);
			break;
		case FdType::FCLASS:
			double val = cpu.flt_regs[rs1];
			bool is_neg = std::signbit(val);

			switch (std::fpclassify(val)) {
			case FP_INFINITE:
				if (is_neg)
					cpu.int_regs[rd] = FValue::INF;
				else
					cpu.int_regs[rd] = FValue::POS_INF;
				break;
			case FP_NORMAL:
				if (is_neg)
					cpu.int_regs[rd] = FValue::NORMAL;
				else
					cpu.int_regs[rd] = FValue::POS_NORMAL;
				break;
			case FP_SUBNORMAL:
				if (is_neg)
					cpu.int_regs[rd] = FValue::SUBNORMAL;
				else
					cpu.int_regs[rd] = FValue::POS_SUBNORMAL;
				break;
			case FP_ZERO:
				if (is_neg)
					cpu.int_regs[rd] = FValue::NEG_ZERO;
				else
					cpu.int_regs[rd] = FValue::POS_ZERO;
				break;
			case FP_NAN:
				if (is_snand(val))
					cpu.int_regs[rd] = FValue::NAN_SIG;
				else
					cpu.int_regs[rd] = FValue::NAN_QUIET;
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
	case FdType::FMVWX:
		cpu.flt_regs[rd] = reinterpret_cast<double>(
			static_cast<uint64_t>(
				static_cast<uint32_t>(
					cpu.int_regs[rs1]
				)
			)
		);
		break;
	case FdType::FMVDX:
		cpu.flt_regs[rd] = reinterpret_cast<double>(
			cpu.int_regs[rs1]
		);
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
