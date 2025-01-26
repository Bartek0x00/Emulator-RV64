#include <cstdint>
#include <functional>
#include <cstring>
#include <cmath>
#include <cfenv>
#include "settings.hpp"
#include "instruction.hpp"
#include "mmu.hpp"
#include "cpu.hpp"
#include "registers.hpp"

#pragma STDC FENV_ACCESS ON

using namespace Emulator;

namespace Emulator {

namespace Instruction {

namespace C {

static void addi4spn(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t imm = ((decoder.insn >> 1U) & 0x3c0U) | ((decoder.insn >> 7U) & 0x30U) |
                   ((decoder.insn >> 2U) & 0x08U) | ((decoder.insn >> 4U) & 0x04U);

    uint64_t val = cpu->int_regs[IRegs::sp] + imm;

    cpu->int_regs[rd] = val;
}

static void fld(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[rs1] + off;

    cpu->flt_regs[rd].u64 = mmu->load(addr, 64);
}

static void lw(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0x40u) | ((decoder.insn >> 7U) & 0x38U) |
                      ((decoder.insn >> 4U) & 0x04U);

    uint64_t addr = cpu->int_regs[rs1] + off;

    cpu->int_regs[rd] = UCAST<int32_t>(mmu->load(addr, 32));
}

static void ld(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[rs1] + off;

    cpu->int_regs[rd] = mmu->load(addr, 64);
}

static void reserved(Decoder decoder)
{
    cpu->set_exception(
		Exception::ILLEGAL_INSTRUCTION, 
		decoder.insn
	);
}

static void fsd(Decoder decoder)
{
    uint64_t rd = decoder.rd_c();
    uint64_t rs1 = decoder.rs1_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[rs1] + off;
    uint64_t rs1_bits = cpu->flt_regs[rs1].u64;

    mmu->store(addr, rs1_bits, 64);
}

static void sw(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t rs2 = decoder.rs2_c();
    uint64_t off = ((decoder.insn << 1U) & 0x40U) | 
				   ((decoder.insn >> 7U) & 0x38U) |
                   ((decoder.insn >> 4U) & 0x04U);

    uint64_t addr = cpu->int_regs[rs1] + off;
    uint32_t val = cpu->int_regs[rs2];

    mmu->store(addr, val, 32);
}

static void sd(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t rs2 = decoder.rs2_c();
    uint64_t off = ((decoder.insn << 1U) & 0xc0U) | 
				   ((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[rs1] + off;
    uint64_t val = cpu->int_regs[rs2];

    mmu->store(addr, val, 64);
}

static void andi(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20U)
        imm = UCAST<int8_t>(imm | 0xc0U);

    cpu->int_regs[rd] &= imm;
}

static void op3(Decoder decoder)
{
	uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
	uint32_t val2 = (decoder.insn >> 5U) & 0x03U;

	uint64_t rd = decoder.rs1_c();
	uint64_t rs2 = decoder.rs2_c();

	if (!val1) {
		switch (val2) {
		case 0:
			cpu->int_regs[rd] -= cpu->int_regs[rs2];
			break;
		case 1:
			cpu->int_regs[rd] ^= cpu->int_regs[rs2];
			break;
		case 2:
			cpu->int_regs[rd] |= cpu->int_regs[rs2];
			break;
		case 3:
			cpu->int_regs[rd] &= cpu->int_regs[rs2];
			break;
		default:
			break;
		}
	} else {
		switch (val2) {
		case 0:
			cpu->int_regs[rd] = UCAST<int32_t>(
				cpu->int_regs[rd] - cpu->int_regs[rs2]
			);
			break;
		case 1:
			cpu->int_regs[rd] = UCAST<int32_t>(
				cpu->int_regs[rd] + cpu->int_regs[rs2]
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}			
	}
}

static void addi(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = UCAST<int8_t>(imm | 0xc0U);

    cpu->int_regs[rd] += imm;
}

static void addiw(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = UCAST<int8_t>(imm | 0xc0U);

    cpu->int_regs[rd] = UCAST<int32_t>(cpu->int_regs[rd] + imm);
}

static void li(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn >> 7U) & 0x20U) | 
				   ((decoder.insn >> 2U) & 0x1fU);

    if (imm & 0x20)
        imm = UCAST<int8_t>(imm | 0xc0U);

    cpu->int_regs[rd] = imm;
}

static void addi16sp(Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 3U) & 0x200U) | 
				   ((decoder.insn >> 2U) & 0x10U) |
                   ((decoder.insn << 1U) & 0x40U) | 
				   ((decoder.insn << 4U) & 0x180U) |
                   ((decoder.insn << 3U) & 0x20U);

    if (imm & 0x200)
        imm = UCAST<int16_t>(imm | 0xfc00U);

    cpu->int_regs[IRegs::sp] += imm;
}

static void lui(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t imm = ((decoder.insn << 5U) & 0x20000U) | 
				   ((decoder.insn << 10U) & 0x1f000U);

    if (imm & 0x20000U)
        imm = UCAST<int32_t>(imm | 0xfffc0000U);

    cpu->int_regs[rd] = imm;
}

static void srli(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t shamt = decoder.shamt_c();

    cpu->int_regs[rd] >>= shamt;
}

static void srai(Decoder decoder)
{
    uint64_t rd = decoder.rs1_c();
    uint64_t shamt = decoder.shamt_c();

    cpu->int_regs[rd] = static_cast<int64_t>(cpu->int_regs[rd]) >> shamt;
}

static void j(Decoder decoder)
{
    uint64_t imm = ((decoder.insn >> 1U) & 0x800U) | ((decoder.insn << 2U) & 0x400U) |
                   ((decoder.insn >> 1U) & 0x300U) | ((decoder.insn << 1U) & 0x80U) |
                   ((decoder.insn >> 1U) & 0x40U) | ((decoder.insn << 3U) & 0x20U) |
                   ((decoder.insn >> 7U) & 0x10U) | ((decoder.insn >> 2U) & 0xeU);

    if (imm & 0x800U)
        imm = UCAST<int16_t>(imm | 0xf000U);

    cpu->pc += (imm - 2);
}

static void beqz(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if (imm & 0x100)
        imm = UCAST<int16_t>(imm | 0xfe00U);

    if (!cpu->int_regs[rs1])
        cpu->pc += (imm - 2);
}

static void bnez(Decoder decoder)
{
    uint64_t rs1 = decoder.rs1_c();
    uint64_t imm = ((decoder.insn >> 4U) & 0x100U) | ((decoder.insn << 1U) & 0xc0U) |
                   ((decoder.insn << 3U) & 0x20U) | ((decoder.insn >> 7U) & 0x18U) |
                   ((decoder.insn >> 2U) & 0x6U);

    if (imm & 0x100)
        imm = UCAST<int16_t>(imm | 0xfe00U);

    if (cpu->int_regs[rs1])
        cpu->pc += (imm - 2);
}

static void slli(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t shamt = decoder.shamt_c();

    cpu->int_regs[rd] <<= shamt;
}

static void fldsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0x1c0U) | 
				   ((decoder.insn >> 7U) & 0x20U) |
                   ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = mmu->load(
		cpu->int_regs[IRegs::sp] + off, 
		64
	);

    cpu->flt_regs[rd].u64 = val;
}

static void lwsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0xc0U) | 
				   ((decoder.insn >> 7U) & 0x20U) |
                   ((decoder.insn >> 2U) & 0x1cU);

    uint32_t val = mmu->load(
		cpu->int_regs[IRegs::sp] + off, 
		32
	);

    cpu->int_regs[rd] = UCAST<int32_t>(val);
}

static void ldsp(Decoder decoder)
{
    uint64_t rd = decoder.rd();
    uint64_t off = ((decoder.insn << 4U) & 0x1c0U) | 
				   ((decoder.insn >> 7U) & 0x20U) |
                   ((decoder.insn >> 2U) & 0x18U);

    uint64_t val = mmu->load(
		cpu->int_regs[IRegs::sp] + off, 
		64
	);

    cpu->int_regs[rd] = val;
}

static void op4(Decoder decoder)
{
	uint32_t val1 = (decoder.insn >> 12U) & 0x01U;
	uint32_t val2 = (decoder.insn >> 2U) & 0x1fU;

	uint64_t rd = decoder.rd();
	uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;

	if (val1) {
		if (val2)
			cpu->int_regs[rd] += cpu->int_regs[rs1];
		else {
			uint64_t tmp = cpu->pc + 2;
			cpu->pc = (cpu->int_regs[rd] - 2);
			cpu->int_regs[IRegs::ra] = tmp;
		}	
	} else {
		if (val2)
			cpu->int_regs[rd] = cpu->int_regs[rs1];
		else
			cpu->pc = cpu->int_regs[rd] - 2;
	}
}

static void fsdsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0x1c0U) | 
					((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[IRegs::sp] + off;
    uint64_t rs1_bits = cpu->flt_regs[rs1].u64;

    mmu->store(addr, rs1_bits, 64);
}

static void swsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0xc0U) | 
					((decoder.insn >> 7U) & 0x3cU);

    uint64_t addr = cpu->int_regs[IRegs::sp] + off;
    uint32_t val = cpu->int_regs[rs1];

    mmu->store(addr, val, 32);
}

static void sdsp(Decoder decoder)
{
    uint64_t rs1 = (decoder.insn >> 2U) & 0x1fU;
    uint64_t off = ((decoder.insn >> 1U) & 0x1c0U) | 
					((decoder.insn >> 7U) & 0x38U);

    uint64_t addr = cpu->int_regs[IRegs::sp] + off;
    uint64_t val = cpu->int_regs[rs1];

    mmu->store(addr, val, 64);
}

static void quadrant0(Decoder decoder)
{
	switch (decoder.funct3_c()) {
	case Decoder::Q0::ADDI4SPN:
		C::addi4spn(decoder);
		break;
	case Decoder::Q0::FLD:
		C::fld(decoder);
		break;
	case Decoder::Q0::LW:
		C::lw(decoder);
		break;
	case Decoder::Q0::LD:
		C::ld(decoder);
		break;
	case Decoder::Q0::RESERVED:
		C::reserved(decoder);
		break;
	case Decoder::Q0::FSD:
		C::fsd(decoder);
		break;
	case Decoder::Q0::SW:
		C::sw(decoder);
		break;
	case Decoder::Q0::SD:
		C::sd(decoder);
		break;
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
	}
}

static void quadrant1(Decoder decoder)
{
	switch (decoder.funct3_c()) {
	case Decoder::Q1::ADDI:
		C::addi(decoder);
		break;
	case Decoder::Q1::ADDIW:
		C::addiw(decoder);
		break;
	case Decoder::Q1::LI:
		C::li(decoder);
		break;
	case Decoder::Q1::OP03:
		switch (decoder.rd()) {
		case Decoder::Q1::NOP:
			break;
		case Decoder::Q1::ADDI16SP:
			C::addi16sp(decoder);
			break;
		default:
			C::lui(decoder);
			break;
		}
		break;
	case Decoder::Q1::OP04:
		switch (decoder.funct2_c()) {
		case Decoder::Q1::SRLI:
			C::srli(decoder);
			break;
		case Decoder::Q1::SRAI:
			C::srai(decoder);
			break;
		case Decoder::Q1::ANDI:
			C::andi(decoder);
			break;
		case Decoder::Q1::OP03:
			C::op3(decoder);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::Q1::J:
		C::j(decoder);
		break;
	case Decoder::Q1::BEQZ:
		C::beqz(decoder);
		break;
	case Decoder::Q1::BNEZ:
		C::bnez(decoder);
		break;
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

static void quadrant2(Decoder decoder)
{
	switch (decoder.funct3_c()) {
	case Decoder::Q2::SLLI:
		C::slli(decoder);
		break;
	case Decoder::Q2::FLDSP:
		C::fldsp(decoder);
		break;
	case Decoder::Q2::LWSP:
		C::lwsp(decoder);
		break;
	case Decoder::Q2::LDSP:
		C::ldsp(decoder);
		break;
	case Decoder::Q2::OP04:
		C::op4(decoder);
		break;
	case Decoder::Q2::FSDSP:
		C::fsdsp(decoder);
		break;
	case Decoder::Q2::SWSP:
		C::swsp(decoder);
		break;
	case Decoder::Q2::SDSP:
		C::sdsp(decoder);
		break;
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
		break;
	}
}

}; // namespace C

namespace LD {

static inline void ASSIGN_IF_NO_EXC(uint64_t rd, uint64_t value)
{
	uint64_t tmp = value;
	if (cpu->exception.current == Exception::NONE)
		cpu->int_regs[rd] = value;
}

static void funct3(Decoder decoder)
{
	uint64_t rs1 = decoder.rs1();
	uint64_t rd = decoder.rd();
	int64_t imm = decoder.imm_i();
	
	uint64_t addr = cpu->int_regs[rs1] + imm;

	switch (decoder.funct3()) {
	case Decoder::LdType::LB:
		ASSIGN_IF_NO_EXC(
			rd,
			SCAST<int8_t>(
				mmu->load(addr, 8)
			)
		);
		break;
	case Decoder::LdType::LH:
		ASSIGN_IF_NO_EXC(
			rd,
			SCAST<int16_t>(
				mmu->load(addr, 16)
			)
		);
		break;
	case Decoder::LdType::LW:
		ASSIGN_IF_NO_EXC(
			rd,
			SCAST<int32_t>(
				mmu->load(addr, 32)
			)
		);
		break;
	case Decoder::LdType::LD:		
		ASSIGN_IF_NO_EXC(
			rd,
			SCAST<int64_t>(
				mmu->load(addr, 64)
			)
		);
		break;
	case Decoder::LdType::LBU:
		ASSIGN_IF_NO_EXC(
			rd,
			mmu->load(addr, 8)
		);
		break;
	case Decoder::LdType::LHU:
		ASSIGN_IF_NO_EXC(
			rd,
			mmu->load(addr, 16)
		);
		break;
	case Decoder::LdType::LWU:
		ASSIGN_IF_NO_EXC(
			rd,
			mmu->load(addr, 32)
		);
		break;
	default:
		cpu->set_exception(
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

	uint64_t addr = cpu->int_regs[rs1] + imm;
	uint64_t value = cpu->int_regs[rs2];

	switch (decoder.funct3()) {
	case Decoder::StType::SB:
		mmu->store(addr, value, 8);
		break;
	case Decoder::StType::SH:
		mmu->store(addr, value, 16);
		break;
	case Decoder::StType::SW:
		mmu->store(addr, value, 32);
		break;
	case Decoder::StType::SD:
		mmu->store(addr, value, 64);
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
	
	uint64_t uval1 = cpu->int_regs[rs1];
	uint64_t uval2 = cpu->int_regs[rs2];

	int64_t val1 = static_cast<int64_t>(uval1);
	int64_t val2 = static_cast<int64_t>(uval2);
	
	__int128 lval1 = val1;
	__int128 lval2 = val2;
	
	unsigned __int128 ulval1 = uval1;
	unsigned __int128 ulval2 = uval2;

	switch (decoder.funct3()) {
	case Decoder::RType::ADDMULSUB:
		switch (decoder.funct7()) {
		case Decoder::RType::ADD:
			cpu->int_regs[rd] = val1 + val2;
			break;
		case Decoder::RType::MUL:
			cpu->int_regs[rd] = val1 * val2;
			break;
		case Decoder::RType::SUB:
			cpu->int_regs[rd] = val1 - val2;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::SLLMULH:
		switch (decoder.funct7()) {
		case Decoder::RType::SLL:
			cpu->int_regs[rd] = val1 << val2;
			break;
		case Decoder::RType::MULH:
			cpu->int_regs[rd] = (lval1 * lval2) >> 64;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::SLTMULHSU:
		switch (decoder.funct7()) {
		case Decoder::RType::SLT:
			cpu->int_regs[rd] = val1 < val2;
			break;
		case Decoder::RType::MULHSU:
			ulval1 = static_cast<__int128>(val1);
			cpu->int_regs[rd] = (ulval1 * ulval2) >> 64;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::SLTUMULHU:
		switch (decoder.funct7()) {
		case Decoder::RType::SLTU:
			cpu->int_regs[rd] = uval1 < uval2;
			break;
		case Decoder::RType::MULHU:
			cpu->int_regs[rd] = (ulval1 * ulval2) >> 64;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::XORDIV:
		switch (decoder.funct7()) {
		case Decoder::RType::XOR:
			cpu->int_regs[rd] = uval1 ^ uval2;
			break;
		case Decoder::RType::DIV:
			switch (val2) {
			case -1:
				if (val1 == std::numeric_limits<int32_t>::min())
					cpu->int_regs[rd] = val1;
				break;
			case 0:
				cpu->int_regs[rd] = ~0ULL;
				break;
			default:
				cpu->int_regs[rd] = val1 / val2;
				break;
			}
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::SR:
		switch (decoder.funct7()) {
		case Decoder::RType::SRL:
			cpu->int_regs[rd] = uval1 >> uval2;
			break;
		case Decoder::RType::DIVU:
			if (val2)
				cpu->int_regs[rd] = uval1 / uval2;
			else
				cpu->int_regs[rd] = ~0ULL;
			break;
		case Decoder::RType::SRA:
		{
			int32_t hval1 = uval1;
			cpu->int_regs[rd] = hval1 >> val2;
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::ORREM:
		switch (decoder.funct7()) {
		case Decoder::RType::OR:
			cpu->int_regs[rd] = uval1 | uval2;
			break;
		case Decoder::RType::REM:
			switch (val2) {
			case -1:
				if (val1 == std::numeric_limits<int64_t>::min())
					cpu->int_regs[rd] = 0;
				break;
			case 0:
				cpu->int_regs[rd] = val1;
				break;
			default:
				cpu->int_regs[rd] = val1 % val2;
				break;
			}
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::RType::ANDREM:
		switch (decoder.funct7()) {
		case Decoder::RType::AND:
			cpu->int_regs[rd] = uval1 & uval2;
			break;
		case Decoder::RType::REMU:
			if (val2)
				cpu->int_regs[rd] = uval1 % uval2;
			else
				cpu->int_regs[rd] = uval1;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	default:
		cpu->set_exception(
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

	switch (decoder.funct3()) {
	case Decoder::R64Type::ADDSUBW:
	{
		uint64_t val1 = cpu->int_regs[rs1];
		int64_t val2 = cpu->int_regs[rs2];

		switch (decoder.funct7()) {
		case Decoder::R64Type::ADDW:
			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 + val2
			);
			break;
		case Decoder::R64Type::MULW:
			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 * val2
			);
			break;
		case Decoder::R64Type::SUBW:
			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 - val2
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::R64Type::DIVW:
	{
		int32_t val1 = cpu->int_regs[rs1];
		int32_t val2 = cpu->int_regs[rs2];

		switch (val2) {
		case -1:
			if (val1 == std::numeric_limits<int32_t>::min())
				cpu->int_regs[rd] = static_cast<int64_t>(val1);
			break;
		case 0:
			cpu->int_regs[rd] = ~0ULL;
			break;
		default:
			cpu->int_regs[rd] = UCAST<int64_t>(val1 / val2);
			break;
		}
		break;
	}
	case Decoder::R64Type::SLLW:
	{
		uint64_t val1 = cpu->int_regs[rs1];
		uint64_t val2 = cpu->int_regs[rs2] & 0x1f;

		cpu->int_regs[rd] = UCAST<int32_t>(
			val1 << val2
		);
		break;
	}
	case Decoder::R64Type::SRW:
		switch (decoder.funct7()) {
		case Decoder::R64Type::SRLW:
		{
			uint32_t val1 = cpu->int_regs[rs1];
			uint64_t val2 = cpu->int_regs[rs2] & 0x1f;

			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 >> val2
			);
			break;
		}
		case Decoder::R64Type::DIVUW:
		{
			uint32_t val1 = cpu->int_regs[rs1];
			uint32_t val2 = cpu->int_regs[rs2];

			if (val2)
				cpu->int_regs[rd] = UCAST<int32_t>(
					val1 / val2
				);
			else
				cpu->int_regs[rd] = ~0ULL;

			break;
		}
		case Decoder::R64Type::SRAW:
		{
			int32_t val1 = cpu->int_regs[rs1];
			uint64_t val2 = cpu->int_regs[rs2] & 0x1f;

			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 >> val2
			);
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::R64Type::REMW:
	{
		int64_t val1 = static_cast<int32_t>(
			cpu->int_regs[rs1]
		);
		int64_t val2 = cpu->int_regs[rs2];
		
		if (val2)
			cpu->int_regs[rd] = UCAST<int32_t>(
				val1 % val2
			);
		else
			cpu->int_regs[rd] = static_cast<uint64_t>(
				val1
			);
		break;
	}
	case Decoder::R64Type::REMUW:
	{
		uint64_t val1 = cpu->int_regs[rs1];
		int64_t val2 = cpu->int_regs[rs2];
		
		if (val2)
			cpu->int_regs[rd] = val1 % val2;
		else
			cpu->int_regs[rd] = val1;
		break;
	}
	default:
		cpu->set_exception(
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
	uint64_t uimm = decoder.imm_i();
	uint32_t shamt = decoder.shamt();
	
	switch (decoder.funct3()) {
	case Decoder::IType::ADDI:
		cpu->int_regs[rd] = cpu->int_regs[rs1] + imm;
		break;
	case Decoder::IType::SLLI:
		cpu->int_regs[rd] = cpu->int_regs[rs1] << shamt;
		break;
	case Decoder::IType::SLTI:
		cpu->int_regs[rd] = static_cast<int64_t>(
			cpu->int_regs[rs1]
		) < imm;
		break;
	case Decoder::IType::SLTIU:
		cpu->int_regs[rd] = cpu->int_regs[rs1] < uimm;
		break;
	case Decoder::IType::XORI:
		cpu->int_regs[rd] = cpu->int_regs[rs1] ^ uimm;
		break;
	case Decoder::IType::SRI:
		switch (decoder.funct7() >> 1) {
		case Decoder::IType::SRLI:
			cpu->int_regs[rd] = cpu->int_regs[rs1] >> shamt;
			break;
		case Decoder::IType::SRAI:
			cpu->int_regs[rd] = static_cast<int64_t>(cpu->int_regs[rs1]) >> shamt;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::IType::ORI:
		cpu->int_regs[rd] = cpu->int_regs[rs1] | uimm;
		break;
	case Decoder::IType::ANDI:
		cpu->int_regs[rd] = cpu->int_regs[rs1] & uimm;
		break;
	default:
		cpu->set_exception(
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

	switch (decoder.funct3()) {
	case Decoder::I64Type::ADDIW:
	{
		int64_t imm = decoder.imm_i();
		cpu->int_regs[rd] = UCAST<int32_t>(
			cpu->int_regs[rs1] + imm
		);
		break;
	}
	case Decoder::I64Type::SLLIW:
	{
		uint64_t shamt = decoder.shamt();
		cpu->int_regs[rd] = UCAST<int32_t>(
			cpu->int_regs[rs1] << shamt
		);
		break;
	}
	case Decoder::I64Type::SRIW:
	{
		uint32_t shamt = decoder.shamt();
		switch (decoder.funct7()) {
		case Decoder::I64Type::SRLIW:
			cpu->int_regs[rd] = UCAST<int32_t>(
				static_cast<uint32_t>(cpu->int_regs[rs1]) >> shamt
			);
			break;
		case Decoder::I64Type::SRAIW:
			cpu->int_regs[rd] = UCAST<int64_t>(
				static_cast<int32_t>(cpu->int_regs[rs1]) >> shamt
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
}

}; // namespace I64

namespace FD {

struct FValue {
	enum : uint64_t {
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
};

static void set_exceptions(void)
{
	int32_t exceptions = std::fetestexcept(FE_ALL_EXCEPT);
	uint64_t current = 0;

	if (exceptions & FE_DIVBYZERO)
		current = CRegs::FExcept::DIVBYZERO;
	
	if (exceptions & FE_INEXACT)
		current = CRegs::FExcept::INEXACT;
	
	if (exceptions & FE_OVERFLOW)
		current = CRegs::FExcept::OVERFLOW;

	if (exceptions & FE_UNDERFLOW)
		current = CRegs::FExcept::UNDERFLOW;
	
	if (current)
		cpu->csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu->csr_regs.load(CRegs::Address::FFLAGS) |
				current
		);
	
	if (exceptions)
		std::feclearexcept(FE_ALL_EXCEPT);
}

static inline bool is_fs(void)
{
	uint64_t fs_bits = read_bits(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		),
		14, 13
	);

	if (fs_bits != CRegs::FS::OFF)
		return true;
	
	cpu->set_exception(
		Exception::ILLEGAL_INSTRUCTION
	);
	return false;
}

static inline float get_cnanf(void)
{
	uint32_t nan_raw = 0x7fc00000U;
	return std::bit_cast<float>(nan_raw);
}

static inline double get_cnand(void)
{
	uint64_t nan_raw = 0x7ff8000000000000ULL;
	return std::bit_cast<double>(nan_raw);
}

static inline bool is_cnanf(float valf)
{
	uint32_t nan_raw = 0x7fc00000U;
	uint32_t val = std::bit_cast<uint32_t>(valf);
	return nan_raw == (val & 0x7fffffffU);
}

static inline bool is_cnand(double vald)
{
	uint64_t nan_raw = 0x7ff8000000000000ULL;
	uint64_t val = std::bit_cast<uint64_t>(vald);
	return nan_raw == (val & 0x7fffffffffffffffULL);
}

static inline bool is_snanf(float valf)
{
	uint32_t snan_raw = 0x7f800001U;
	uint32_t val = std::bit_cast<uint32_t>(valf);
	return snan_raw == (val & 0x7fffffffU);
}

static inline bool is_snand(double vald)
{
	uint64_t snan_raw = 0x7ff0000000000001ULL;
	uint64_t val = std::bit_cast<uint64_t>(vald);
	return snan_raw == (val & 0x7fffffffffffffffULL);
}

template<typename T>
static T round(T val, Decoder decoder)
{
	T newval = val;

	switch (decoder.rounding_mode()) {
	case Cpu::FPURoundingMode::ROUND_TO_NEAREST:
	case Cpu::FPURoundingMode::ROUND_NEAREST_MAX_MAGNITUDE:
		newval = std::round(val);
		break;
	case Cpu::FPURoundingMode::ROUND_TO_ZERO:
		newval = std::trunc(val);
		break;
	case Cpu::FPURoundingMode::ROUND_DOWN:
		newval = std::floor(val);
		break;
	case Cpu::FPURoundingMode::ROUND_UP:
		newval = std::ceil(val);
		break;
	case Cpu::FPURoundingMode::ROUND_DYNAMIC:
		break;
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION
		);
		break;
	}

	if (newval != val)
		cpu->csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu->csr_regs.load(
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
		cpu->csr_regs.store(
			CRegs::Address::FFLAGS,
			cpu->csr_regs.load(
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

	uint64_t addr = cpu->int_regs[rs1] + off;

	switch (decoder.funct3()) {
	case Decoder::FdType::FLW:
	{
		uint32_t tmp = mmu->load(addr, 32);
		uint64_t val = tmp | 0xffffffff00000000ULL;
		cpu->flt_regs[rd].u64 = val;
		break;
	}
	case Decoder::FdType::FLD:
	{
		uint64_t val = mmu->load(addr, 64);
		cpu->flt_regs[rd].u64 = val;
		break;
	}
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}

	set_exceptions();
}

static void fs(Decoder decoder)
{
	if (!is_fs())
		return;
	
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t off = decoder.fs_off();

	uint64_t addr = cpu->int_regs[rs1] + off;

	switch (decoder.funct3()) {
	case Decoder::FdType::FSW:
		mmu->store(addr, cpu->flt_regs[rs2].u32, 32);
		break;
	case Decoder::FdType::FSD:
		mmu->store(addr, cpu->flt_regs[rs2].u64, 64);
		break;
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		break;
	}
	set_exceptions();
}

static void fmadd(Decoder decoder)
{
	if (!is_fs())
		return;
	
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	uint64_t rs2 = decoder.rs2();
	uint64_t rs3 = decoder.rs3();

	switch (decoder.funct2()) {
	case Decoder::FdType::FMADDS:
	{
		float val1 = cpu->flt_regs[rs1].f;
		float val2 = cpu->flt_regs[rs2].f;
		float val3 = cpu->flt_regs[rs3].f;

		float res = (val1 * val2) + val3;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
		
		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FMADDD:
	{
		double val1 = cpu->flt_regs[rs1].d;
		double val2 = cpu->flt_regs[rs2].d;
		double val3 = cpu->flt_regs[rs3].d;

		double res = (val1 * val2) + val3;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	default:
		cpu->set_exception(
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

	switch (decoder.funct2()) {
	case Decoder::FdType::FMSUBS:
	{
		float val1 = cpu->flt_regs[rs1].f;
		float val2 = cpu->flt_regs[rs2].f;
		float val3 = cpu->flt_regs[rs3].f;

		float res = (val1 * val2) - val3;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FMSUBD:
	{
		double val1 = cpu->flt_regs[rs1].d;
		double val2 = cpu->flt_regs[rs2].d;
		double val3 = cpu->flt_regs[rs3].d;

		double res = (val1 * val2) - val3;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	default:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
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

	switch (decoder.funct2()) {
	case Decoder::FdType::FNMADDS:
	{
		float val1 = -cpu->flt_regs[rs1].f;
		float val2 = cpu->flt_regs[rs2].f;
		float val3 = cpu->flt_regs[rs3].f;

		float res = (val1 * val2) + val3;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FNMADDD:
	{
		double val1 = -cpu->flt_regs[rs1].d;
		double val2 = cpu->flt_regs[rs2].d;
		double val3	= cpu->flt_regs[rs3].d;

		double res = (val1 * val2) + val3;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	default:
		cpu->set_exception(
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

	switch (decoder.funct2()) {
	case Decoder::FdType::FNMSUBS:
	{
		float val1 = -cpu->flt_regs[rs1].f;
		float val2 = cpu->flt_regs[rs2].f;
		float val3 = cpu->flt_regs[rs3].f;

		float res = (val1 * val2) - val3;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FNMSUBD:
	{
		double val1 = -cpu->flt_regs[rs1].d;
		double val2 = cpu->flt_regs[rs2].d;
		double val3 = cpu->flt_regs[rs3].d;

		double res = (val1 * val2) - val3;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	default:
		cpu->set_exception(
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

	float val1f = cpu->flt_regs[rs1].f;
	float val2f = cpu->flt_regs[rs2].f;

	double val1d = cpu->flt_regs[rs1].d;
	double val2d = cpu->flt_regs[rs2].d;

	switch (decoder.funct7()) {
	case Decoder::FdType::FADDS:
	{
		float res = val1f + val2f;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FADDD:
	{
		double res = val1d + val2d;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	case Decoder::FdType::FSUBS:
	{
		FRegs::FProxy res;
		res.f = val1f - val2f;

		if (is_cnanf(res.f)) {
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
			
			res.u32 &= 0x7fffffffU;
		}

		if (is_snanf(res.f))
			res.u32 &= 0x7fffffffU;
		
		cpu->flt_regs[rd].u32 = res.u32;
		break;
	}
	case Decoder::FdType::FSUBD:
	{
		FRegs::FProxy res;
		res.d = val1d - val2d;

		if (is_cnand(res.d)) {
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
			
			res.u64 &= 0x7fffffffffffffffULL;
		}

		if (is_snand(res.d))
			res.u64 &= 0x7fffffffffffffffULL;
		
		cpu->flt_regs[rd].u64 = res.u64;
		break;
	}
	case Decoder::FdType::FMULS:
	{
		float res = val1f * val2f;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FMULD:
	{
		double res = val1d * val2d;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	case Decoder::FdType::FDIVS:
	{
		float res = val1f / val2f;
		if (is_cnanf(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].f = res;
		break;
	}
	case Decoder::FdType::FDIVD:
	{
		double res = val1d / val2d;
		if (is_cnand(res))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		cpu->flt_regs[rd].d = res;
		break;
	}
	case Decoder::FdType::FSNGJS:
		switch (decoder.funct3()) {
		case Decoder::FdType::FSGNJ:
			cpu->flt_regs[rd].f = static_cast<float>(
				std::copysign(val1f, val2f)
			);
			break;
		case Decoder::FdType::FSGNJN:
			val2f = -cpu->flt_regs[rs2].f;
			cpu->flt_regs[rd].f = static_cast<float>(
				std::copysign(val1f, val2f)
			);
			break;
		case Decoder::FdType::FSGNJX:
		{
			uint32_t uval1 = cpu->flt_regs[rs1].u32;
			uint32_t uval2 = cpu->flt_regs[rs2].u32;

			uint32_t mask = 1ULL << 31;
			uint32_t res = uval1 & (mask - 1);
			res |= (uval1 & mask) ^ (uval2 & mask);
			
			cpu->flt_regs[rd].u32 = res;
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn			
			);
			break;
		}
		break;
	case Decoder::FdType::FSNGJD:
		switch (decoder.funct3()) {
		case Decoder::FdType::FSGNJ:
			cpu->flt_regs[rd].d = static_cast<double>(
				std::copysign(val1d, val2d)
			);
			break;
		case Decoder::FdType::FSGNJN:
			val2d = -cpu->flt_regs[rs2].d;
			cpu->flt_regs[rd].d = static_cast<double>(
				std::copysign(val1d, val2d)
			);
			break;
		case Decoder::FdType::FSGNJX:
		{
			uint64_t uval1 = cpu->flt_regs[rs1].u64;
			uint64_t uval2 = cpu->flt_regs[rs2].u64;

			uint64_t mask = 1ULL << 63;
			uint64_t res = uval1 & (mask - 1);
			res |= (uval1 & mask) ^ (uval2 & mask);

			cpu->flt_regs[rd].u64 = res;
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::FdType::FMINMAXS:
	{
		bool is_val1_nan = std::isnan(val1f);
		bool is_val2_nan = std::isnan(val2f);

		if (is_snanf(val1f) || is_snanf(val2f))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		if (is_val1_nan && is_val2_nan) {
			cpu->flt_regs[rd].f = get_cnanf();
			break;
		}

		if (is_val1_nan) {
			cpu->flt_regs[rd].f = val2f;
			break;
		}

		if (is_val2_nan) {
			cpu->flt_regs[rd].f = val1f;
			break;
		}

		switch (decoder.funct3()) {
		case Decoder::FdType::MIN:
			if (val1f != val2f)
				cpu->flt_regs[rd].f = std::min(
					val1f, val2f
				);
			else
				cpu->flt_regs[rd].f = std::signbit(val1f) ? val1f : val2f;
			break;
		case Decoder::FdType::MAX:
			if (val1f != val2f)
				cpu->flt_regs[rd].f = std::max(
					val1f, val2f
				);
			else
				cpu->flt_regs[rd].f = std::signbit(val1f) ? val2f : val1f;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::FdType::FMINMAXD:
	{
		bool is_val1_nan = std::isnan(val1d);
		bool is_val2_nan = std::isnan(val2d);
		
		if (is_snand(val1d) || is_snand(val2d))
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);

		if (is_val1_nan && is_val2_nan) {
			cpu->flt_regs[rd].d = get_cnand();
			break;
		}

		if (is_val1_nan) {
			cpu->flt_regs[rd].d = val2d;
			break;
		}

		if (is_val2_nan) {
			cpu->flt_regs[rd].d = val1d;
			break;
		}
		
		switch (decoder.funct3()) {
		case Decoder::FdType::MIN:
			if (val1d != val2d)
				cpu->flt_regs[rd].d = std::min(val1d, val2d);
			else
				cpu->flt_regs[rd].d = std::signbit(val1d) ? val1d : val2d;
			break;
		case Decoder::FdType::MAX:
			if (val1d != val2d)
				cpu->flt_regs[rd].d = std::max(val1d, val2d);
			else
				cpu->flt_regs[rd].d = std::signbit(val1d) ? val2d : val1d;
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		}
		break;
	}
	case Decoder::FdType::FCVTSD:
	{
		float val = cpu->flt_regs[rs1].d;
		if (std::isnan(val))
			val = std::numeric_limits<float>::quiet_NaN();
		
		cpu->flt_regs[rd].d = 0.0;	
		cpu->flt_regs[rd].f = val;
		break;
	}
	case Decoder::FdType::FCVTDS:
	{
		double val = cpu->flt_regs[rs1].f;
		if (std::isnan(val))
			val = std::numeric_limits<double>::quiet_NaN();
		
		cpu->flt_regs[rd].d = val;
		break;
	}
	case Decoder::FdType::FSQRTS:
		if (val1f < 0.0f) {
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
			cpu->flt_regs[rd].f = 
				std::bit_cast<float>(0x7fc00000U);
		} else 
			cpu->flt_regs[rd].f = std::sqrt(val1f);
		break;
	case Decoder::FdType::FSQRTD:
		if (val1d < 0.0) {
			cpu->csr_regs.store(
				CRegs::Address::FFLAGS,
				cpu->csr_regs.load(
					CRegs::Address::FFLAGS
				) | CRegs::FExcept::INVALID
			);
			cpu->flt_regs[rd].d = 
				std::bit_cast<double>(0x7ff8000000000000ULL);
		} else
			cpu->flt_regs[rd].d = std::sqrt(val1d);
		break;
	case Decoder::FdType::FCS:
		switch (decoder.funct3()) {
		case Decoder::FdType::FLE:
			if (std::isnan(val1f) || std::isnan(val2f))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1f <= val2f
			);
			break;
		case Decoder::FdType::FLT:
			if (std::isnan(val1f) || std::isnan(val2f))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1f < val2f
			);
			break;
		case Decoder::FdType::FEQ:
			if (is_snanf(val1f) || is_snanf(val2f))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1f == val2f
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::FdType::FCD:
		switch (decoder.funct3()) {
		case Decoder::FdType::FLE:
			if (std::isnan(val1d) || std::isnan(val2d))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1d <= val2d
			);
			break;
		case Decoder::FdType::FLT:
			if (std::isnan(val1d) || std::isnan(val2d))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1d < val2d
			);
			break;
		case Decoder::FdType::FEQ:
			if (is_snand(val1d) || is_snand(val2d))
				cpu->csr_regs.store(
					CRegs::Address::FFLAGS,
					cpu->csr_regs.load(
						CRegs::Address::FFLAGS
					) | CRegs::FExcept::INVALID
				);
			cpu->int_regs[rd] = static_cast<bool>(
				val1d == val2d
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::FdType::FCVTS:
	{
		float val = round<float>(
			cpu->flt_regs[rs1].f,
			decoder
		);
		switch (rs2) {
		case Decoder::FdType::FCVT0:
			cpu->int_regs[rd] = UCAST<int32_t>(
				try_convert<int32_t>(val)
			);
			break;
		case Decoder::FdType::FCVT1:
			cpu->int_regs[rd] = UCAST<int32_t>(
				try_convert<uint32_t>(val)
			);
			break;
		case Decoder::FdType::FCVT2:
			cpu->int_regs[rd] = try_convert<int64_t>(val);
			break;
		case Decoder::FdType::FCVT3:
			cpu->int_regs[rd] = try_convert<uint64_t>(val);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::FdType::FCVTD:
	{
		double val = round<double>(
			cpu->flt_regs[rs1].d,
			decoder
		);

		switch (rs2) {
		case Decoder::FdType::FCVT0:
			cpu->int_regs[rd] = UCAST<int32_t>(
				try_convert<int32_t>(val)
			);
			break;
		case Decoder::FdType::FCVT1:
			cpu->int_regs[rd] = UCAST<uint32_t>(
				try_convert<uint32_t>(val)
			);
			break;
		case Decoder::FdType::FCVT2:
			cpu->int_regs[rd] = try_convert<int64_t>(val);
			break;
		case Decoder::FdType::FCVT3:
			cpu->int_regs[rd] = try_convert<uint64_t>(val);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::FdType::FCVTSW:
	{
		int64_t val = cpu->int_regs[rs1];
		cpu->flt_regs[rd].d = 0.0;

		switch (rs2) {
		case Decoder::FdType::FCVT0:
			cpu->flt_regs[rd].f = static_cast<int32_t>(val);
			break;
		case Decoder::FdType::FCVT1:
			cpu->flt_regs[rd].f = static_cast<uint32_t>(val);
			break;
		case Decoder::FdType::FCVT2:
			cpu->flt_regs[rd].f = val;
			break;
		case Decoder::FdType::FCVT3:
			cpu->flt_regs[rd].f = static_cast<uint64_t>(val);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::FdType::FCVTDW:
	{
		int64_t val = cpu->int_regs[rs1];

		switch (rs2) {
		case Decoder::FdType::FCVT0:
			cpu->flt_regs[rd].d = static_cast<int32_t>(val);
			break;
		case Decoder::FdType::FCVT1:
			cpu->flt_regs[rd].d = static_cast<uint32_t>(val);
			break;
		case Decoder::FdType::FCVT2:
			cpu->flt_regs[rd].d = val;
			break;
		case Decoder::FdType::FCVT3:
			cpu->flt_regs[rd].d = static_cast<uint64_t>(val);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::FdType::FMVXW:
		switch (decoder.funct3()) {
		case Decoder::FdType::FMV:
		{
			uint32_t val = cpu->flt_regs[rs1].u32;
			cpu->int_regs[rd] = UCAST<int32_t>(val);
			break;
		}
		case Decoder::FdType::FCLASS:
		{
			float val = cpu->flt_regs[rs1].f;
			bool is_neg = std::signbit(val);

			switch (std::fpclassify(val)) {
			case FP_INFINITE:
				if (is_neg)
					cpu->int_regs[rd] = FValue::INF;
				else
					cpu->int_regs[rd] = FValue::POS_INF;
				break;
			case FP_NORMAL:
				if (is_neg)
					cpu->int_regs[rd] = FValue::NORMAL;
				else
					cpu->int_regs[rd] = FValue::POS_NORMAL;
				break;
			case FP_SUBNORMAL:
				if (is_neg)
					cpu->int_regs[rd] = FValue::SUBNORMAL;
				else
					cpu->int_regs[rd] = FValue::POS_SUBNORMAL;
				break;
			case FP_ZERO:
				if (is_neg)
					cpu->int_regs[rd] = FValue::NEG_ZERO;
				else
					cpu->int_regs[rd] = FValue::POS_ZERO;
				break;
			case FP_NAN:
				if (is_snanf(val))
					cpu->int_regs[rd] = FValue::NAN_SIG;
				else
					cpu->int_regs[rd] = FValue::NAN_QUIET;
				break;
			}
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::FdType::FMVXD:
		switch (decoder.funct3()) {
		case Decoder::FdType::FMV:
		{
			uint64_t val = cpu->flt_regs[rs1].u64;
			cpu->int_regs[rd] = UCAST<int64_t>(val);
			break;
		}
		case Decoder::FdType::FCLASS:
		{
			double val = cpu->flt_regs[rs1].d;
			bool is_neg = std::signbit(val);

			switch (std::fpclassify(val)) {
			case FP_INFINITE:
				if (is_neg)
					cpu->int_regs[rd] = FValue::INF;
				else
					cpu->int_regs[rd] = FValue::POS_INF;
				break;
			case FP_NORMAL:
				if (is_neg)
					cpu->int_regs[rd] = FValue::NORMAL;
				else
					cpu->int_regs[rd] = FValue::POS_NORMAL;
				break;
			case FP_SUBNORMAL:
				if (is_neg)
					cpu->int_regs[rd] = FValue::SUBNORMAL;
				else
					cpu->int_regs[rd] = FValue::POS_SUBNORMAL;
				break;
			case FP_ZERO:
				if (is_neg)
					cpu->int_regs[rd] = FValue::NEG_ZERO;
				else
					cpu->int_regs[rd] = FValue::POS_ZERO;
				break;
			case FP_NAN:
				if (is_snand(val))
					cpu->int_regs[rd] = FValue::NAN_SIG;
				else
					cpu->int_regs[rd] = FValue::NAN_QUIET;
				break;
			}
			break;
		}
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::FdType::FMVWX:
		cpu->flt_regs[rd].u64 = static_cast<uint64_t>(
			static_cast<uint32_t>(
				cpu->int_regs[rs1]
			)
		);
		break;
	case Decoder::FdType::FMVDX:
		cpu->flt_regs[rd].u64 = cpu->int_regs[rs1];
		break;
	default:
		cpu->set_exception(
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
	uint64_t val1 = cpu->int_regs[decoder.rs1()];
	uint64_t val2 = cpu->int_regs[decoder.rs2()];
	
	int64_t val1_s = static_cast<int64_t>(val1);
	int64_t val2_s = static_cast<int64_t>(val2);
	int64_t imm = decoder.imm_b() - 4;

	switch (decoder.funct3()) {
	case Decoder::BType::BEQ:
		if (val1_s == val2_s)
			cpu->pc += imm;
		break;
	case Decoder::BType::BNE:
		if (val1_s != val2_s)
			cpu->pc += imm;
		break;
	case Decoder::BType::BLT:
		if (val1_s < val2_s)
			cpu->pc += imm;
		break;
	case Decoder::BType::BGE:
		if (val1_s >= val2_s)
			cpu->pc += imm;
		break;
	case Decoder::BType::BLTU:
		if (val1 < val2)
			cpu->pc += imm;
		break;
	case Decoder::BType::BGEU:
		if (val1 >= val2)
			cpu->pc += imm;
		break;
	default:
		cpu->set_exception(
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
	uint64_t addr = cpu->int_regs[rs1];

	switch (decoder.funct3()) {
	case Decoder::AType::AMOW:
	{
		if (cpu->int_regs[rs1] % 4)
			cpu->set_exception(
				Exception::INSTRUCTION_ADDRESS_MISALIGNED,
				decoder.insn
			);

		int32_t val1 = mmu->load(addr, 32);
		int32_t val2 = cpu->int_regs[rs2];

		switch (decoder.funct5()) {
		case Decoder::AType::ADD:
			mmu->store(addr, static_cast<int32_t>(val1 + val2), 32);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::SWAP:
			mmu->store(addr, val2, 32);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::LR:
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			cpu->reservations.insert(addr);
			break;
		case Decoder::AType::SC:
			if (cpu->reservations.contains(addr)) {
				cpu->reservations.erase(addr);
				mmu->store(
					addr, 
					static_cast<uint32_t>(cpu->int_regs[rs2]), 
					32
				);
				cpu->int_regs[rd] = 0;
			} else {
				cpu->int_regs[rd] = 1;
			}
			break;
		case Decoder::AType::XOR:
			mmu->store(
				addr, 
				UCAST<int32_t>(val1 ^ val2), 
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::OR:
			mmu->store(
				addr,
				UCAST<int32_t>(val1 | val2),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::AND:
			mmu->store(
				addr,
				UCAST<int32_t>(val1 & val2),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::MIN:
			mmu->store(
				addr,
				UCAST<int32_t>(std::min(val1, val2)),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::MAX:
			mmu->store(
				addr,
				UCAST<int32_t>(std::max(val1, val2)),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::MINU:
			mmu->store(
				addr,
				std::min(
					static_cast<uint32_t>(mmu->load(addr, 32)),
					static_cast<uint32_t>(cpu->int_regs[rs2])
				),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		case Decoder::AType::MAXU:
			mmu->store(
				addr,
				std::max(
					static_cast<uint32_t>(mmu->load(addr, 32)),
					static_cast<uint32_t>(cpu->int_regs[rs2])
				),
				32
			);
			cpu->int_regs[rd] = UCAST<int32_t>(val1);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	case Decoder::AType::AMOD:
	{
		if (cpu->int_regs[rs1] % 8)
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		
		int64_t val1 = mmu->load(addr, 64);
		int64_t val2 = cpu->int_regs[rs2];

		switch (decoder.funct5()) {
		case Decoder::AType::ADD:
			mmu->store(
				addr, 
				static_cast<int64_t>(val1 + val2),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::SWAP:
			mmu->store(addr, val2, 64);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::LR:
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			cpu->reservations.insert(addr);
			break;
		case Decoder::AType::SC:
			if (cpu->reservations.contains(addr)) {
				cpu->reservations.erase(addr);
				mmu->store(
					addr,
					static_cast<uint64_t>(cpu->int_regs[rs2]),
					64
				);
				cpu->int_regs[rd] = 0;
			} else {
				cpu->int_regs[rd] = 1;
			}
			break;
		case Decoder::AType::XOR:
			mmu->store(
				addr,
				UCAST<int64_t>(val1 ^ val2),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::OR:
			mmu->store(
				addr,
				UCAST<int64_t>(val1 | val2),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::AND:
			mmu->store(
				addr,
				UCAST<int64_t>(val1 & val2),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::MIN:
			mmu->store(
				addr,
				UCAST<int64_t>(std::min(val1, val2)),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::MAX:
			mmu->store(
				addr,
				UCAST<int64_t>(std::max(val1, val2)),
				64
			);
			cpu->int_regs[rd] = UCAST<int64_t>(val1);
			break;
		case Decoder::AType::MINU:
			mmu->store(
				addr,
				UCAST<uint64_t>(std::min(
					static_cast<uint64_t>(mmu->load(addr, 64)),
					static_cast<uint64_t>(cpu->int_regs[rs2])
				)),
				64
			);
			cpu->int_regs[rd] = UCAST<uint64_t>(val1);
			break;
		case Decoder::AType::MAXU:
			mmu->store(
				addr,
				UCAST<uint64_t>(std::max(
					static_cast<uint64_t>(mmu->load(addr, 64)),
					static_cast<uint64_t>(cpu->int_regs[rs2])
				)),
				64
			);
			cpu->int_regs[rd] = UCAST<uint64_t>(val1);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	}
	default:
		cpu->set_exception(
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
	uint64_t csr_val = cpu->csr_regs.load(csr);

	cpu->csr_regs.store(csr, op(csr_val, rhs));
	cpu->int_regs[rd] = csr_val;
}

static void readonly_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(csr);

	cpu->int_regs[rd] = csr_val;
}

static void priviledged_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	if (cpu->mode == Cpu::Mode::USER) {
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(csr);

	cpu->csr_regs.store(csr, op(csr_val, rhs));
	cpu->int_regs[rd] = csr_val;
}

static void enforced_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(csr);
	uint64_t opval = op(csr_val, rhs);

	if (csr_val != opval) {
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	cpu->int_regs[rd] = csr_val;
}

static void fcsr_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(csr);
	uint64_t fexceptions = cpu->csr_regs.load(
		CRegs::Address::FFLAGS
	) & CRegs::FExcept::MASK;

	uint64_t op_val = op(csr_val | fexceptions, rhs);

	cpu->csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu->csr_regs.load(CRegs::Address::FFLAGS) & 
			~CRegs::FExcept::MASK
	);

	cpu->csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu->csr_regs.load(CRegs::Address::FFLAGS) | 
			(op_val & CRegs::FExcept::MASK)
	);

	cpu->csr_regs.store(
		CRegs::Address::FCSR,
		op_val & 0xff
	);
	cpu->int_regs[rd] = csr_val & 0xff;
}

static void fflags_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(csr);
	uint64_t fexceptions = csr_val & CRegs::FExcept::MASK;
	uint64_t op_val = op(fexceptions, rhs);

	cpu->csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu->csr_regs.load(CRegs::Address::FFLAGS) &
			~CRegs::FExcept::MASK
	);

	cpu->csr_regs.store(
		CRegs::Address::FFLAGS,
		cpu->csr_regs.load(CRegs::Address::FFLAGS) |
			(op_val & CRegs::FExcept::MASK)
	);
	
	uint64_t fcsr = cpu->csr_regs.load(CRegs::Address::FCSR);
	fcsr &= ~CRegs::FExcept::MASK;
	fcsr |= op_val & 0xff;
	
	cpu->csr_regs.store(CRegs::Address::FCSR, fcsr);
	cpu->int_regs[rd] = csr_val & CRegs::FExcept::MASK;
}

static void frm_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t rd = decoder.rd();
	uint64_t csr_val = cpu->csr_regs.load(CRegs::Address::FCSR);
	csr_val >>= 5;
	uint64_t op_val = op(csr_val, rhs);

	cpu->csr_regs.store(
		CRegs::Address::FRM,
		~Cpu::FPURoundingMode::MASK
	);

	cpu->csr_regs.store(
		CRegs::Address::FRM,
		cpu->csr_regs.load(CRegs::Address::FRM) |
			(op_val & Cpu::FPURoundingMode::MASK)
	);

	uint64_t fcsr = cpu->csr_regs.load(CRegs::Address::FCSR);
	fcsr &= ~0xe0;
	fcsr |= op_val << 5;
	fcsr &= 0xff;

	cpu->csr_regs.store(CRegs::Address::FCSR, fcsr);
	cpu->int_regs[rd] = csr_val & Cpu::FPURoundingMode::MASK;
}

static void satp_h(Decoder decoder,
	uint64_t csr, uint64_t rhs, op_t op)
{
	uint64_t tvm = read_bit(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		),
		CRegs::Mstatus::TVM
	);

	if (tvm == 1)
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
	
	default_h(decoder, csr, rhs, op);

	mmu->update();
}

static std::array<handler_t, 4096> csr_handlers = [](void) {
	std::array<handler_t, 4096> tmp;
	std::fill(tmp.begin(), tmp.end(), default_h);
	
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
		cpu->csr_regs.load(CRegs::Address::MSTATUS),
		CRegs::Mstatus::TSR
	);
	
	if ((tsr == 1) || cpu->mode == Cpu::Mode::USER) {
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	cpu->pc = cpu->csr_regs.load(CRegs::Address::SEPC) - 4;
	cpu->mode =	read_bit(
		cpu->csr_regs.load(
			CRegs::Address::SSTATUS
		),
		CRegs::Sstatus::SPP
	);

	if (cpu->mode == Cpu::Mode::USER)
		cpu->csr_regs.store(
			CRegs::Address::MSTATUS,
			write_bit(
				cpu->csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mstatus::MPRV, 
				0
			)
		);

	cpu->csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu->csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Sstatus::SIE,
			read_bit(
				cpu->csr_regs.load(
					CRegs::Address::SSTATUS
				),
				CRegs::Sstatus::SPIE
			)
		)
	);

	cpu->csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu->csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Sstatus::SPIE, 
			1
		)
	);

	cpu->csr_regs.store(
		CRegs::Address::SSTATUS,
		write_bit(
			cpu->csr_regs.load(
				CRegs::Address::SSTATUS
			),
			CRegs::Sstatus::SPP, 
			Cpu::Mode::USER
		)
	);
}

static void mret(Decoder decoder)
{
	if (cpu->mode != Cpu::Mode::MACHINE) {
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}
	
	cpu->pc = cpu->csr_regs.load(CRegs::Address::MEPC) - 4;

	uint64_t mpp_bits = read_bits(
		cpu->csr_regs.load(
			CRegs::Address::MSTATUS
		), 
		12, 11
	);

	switch (mpp_bits) {
	case Cpu::Mode::USER:
	case Cpu::Mode::SUPERVISOR:
	case Cpu::Mode::MACHINE:
		cpu->mode = mpp_bits;
		break;
	default:
		cpu->mode = Cpu::Mode::INVALID;
		break;
	}
	

	if (cpu->mode != Cpu::Mode::MACHINE)
		cpu->csr_regs.store(
			CRegs::Address::MSTATUS,
			write_bit(
				cpu->csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mstatus::MPRV,
				0
			)	
		);
	
	cpu->csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bit(
			cpu->csr_regs.load(
				CRegs::Address::MSTATUS
			),
			CRegs::Mstatus::MIE,
			read_bit(
				cpu->csr_regs.load(
					CRegs::Address::MSTATUS
				),
				CRegs::Mstatus::MPIE
			)
		)
	);

	cpu->csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bit(
			cpu->csr_regs.load(
				CRegs::Address::MSTATUS
			),
			CRegs::Mstatus::MPIE,
			1
		)
	);

	cpu->csr_regs.store(
		CRegs::Address::MSTATUS,
		write_bits(
			cpu->csr_regs.load(
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

	switch (funct7) {
	case Decoder::CSRType::SFENCEVMA7:
	{
		uint64_t tvm = read_bit(
			cpu->csr_regs.load(
				CRegs::Address::MSTATUS
			),
			CRegs::Mstatus::TVM
		);
		
		if (tvm == 1 || cpu->mode == Cpu::Mode::USER)
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
		
		mmu->update();
		return;
	}
	case Decoder::CSRType::HFENCEBVMA7:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	case Decoder::CSRType::HFENCEGVMA7:
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION,
			decoder.insn
		);
		return;
	}

	switch (imm) {
	case Decoder::CSRType::ECALL:
		switch (cpu->mode) {
		case Cpu::Mode::MACHINE:
			cpu->set_exception(
				Exception::ECALL_MMODE,
				cpu->pc
			);
			break;
		case Cpu::Mode::SUPERVISOR:
			cpu->set_exception(
				Exception::ECALL_SMODE,
				cpu->pc
			);
			break;
		case Cpu::Mode::USER:
			cpu->set_exception(
				Exception::ECALL_UMODE,
				cpu->pc
			);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::CSRType::EBREAK:
		cpu->set_exception(
			Exception::BREAKPOINT
		);
		break;
	case Decoder::CSRType::RET:
		switch (funct7) {
		case Decoder::CSRType::SRET7:
			sret(decoder);
			break;
		case Decoder::CSRType::MRET7:
			mret(decoder);
			break;
		case Decoder::CSRType::URET7:
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	case Decoder::CSRType::WFI:
		switch (funct7) {
		case Decoder::CSRType::WFI7:
		#ifndef EMU_DEBUG
			cpu->sleep = true;
		#endif
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION,
				decoder.insn
			);
			break;
		}
		break;
	default:
		cpu->set_exception(
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
	uint64_t rhs = cpu->int_regs[rs1];

	switch (decoder.funct3()) {
	case Decoder::CSRType::ENVIRONMENT:
		environment(decoder);
		break;
	case Decoder::CSRType::CSRW:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return rhs;
			}
		);
		break;
	case Decoder::CSRType::CSRS:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr | rhs;
			}
		);
		break;
	case Decoder::CSRType::CSRC:
		csr_handlers[csr](
			decoder,
			csr,
			rhs,
			+[](uint64_t csr, uint64_t rhs) {
				return csr & ~rhs;
			}
		);
		break;
	case Decoder::CSRType::CSRWI:
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
	case Decoder::CSRType::CSRSI:
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
	case Decoder::CSRType::CSRCI:
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
		cpu->set_exception(
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

	cpu->int_regs[rd] = cpu->pc + 4;
	cpu->pc += imm - 4;
}

static void jalr(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	uint64_t rs1 = decoder.rs1();
	int64_t imm = decoder.imm_i();

	uint64_t tmp = cpu->pc + 4;
	int64_t val = cpu->int_regs[rs1];
	
	cpu->pc = ((val + imm) & ~1U) - 4;
	cpu->int_regs[rd] = tmp;
}

static void auipc(Decoder decoder)
{
	uint64_t rd = decoder.rd();
	int64_t imm = UCAST<int32_t>(
		decoder.insn & 0xfffff000ULL
	);

	cpu->int_regs[rd] = cpu->pc + imm;
}

static void lui(Decoder decoder)
{
	uint64_t rd = decoder.rd();

	cpu->int_regs[rd] = UCAST<int32_t>(
		decoder.insn & 0xfffff000ULL
	);
}

}; // namespace O

uint64_t execute(Decoder decoder)
{
	if (decoder.insn == 0) {
		cpu->set_exception(
			Exception::ILLEGAL_INSTRUCTION, 
			decoder.insn
		);
		error<FAIL>("Illegal instruction");
	}
	
	if (decoder.size() == 2) {
		switch (decoder.opcode_c()) {
		case Decoder::OpcodeType::COMPRESSED_QUANDRANT0: 
			C::quadrant0(decoder); 
			break;
		case Decoder::OpcodeType::COMPRESSED_QUANDRANT1:
			C::quadrant1(decoder);
			break;
		case Decoder::OpcodeType::COMPRESSED_QUANDRANT2: 
			C::quadrant2(decoder); 
			break;
		default:
			cpu->set_exception(Exception::ILLEGAL_INSTRUCTION, decoder.insn);
			break;
		}
	} else {
		switch (decoder.opcode()) {
		case Decoder::OpcodeType::LD: 
			LD::funct3(decoder); 
			break;
		case Decoder::OpcodeType::FENCE:
			O::fence(decoder);
			break;
		case Decoder::OpcodeType::I:
			I::funct3(decoder);
			break;
		case Decoder::OpcodeType::ST:
			ST::funct3(decoder);
			break;
		case Decoder::OpcodeType::R:
			R::funct3(decoder);
			break;
		case Decoder::OpcodeType::B:
			B::funct3(decoder);
			break;
		case Decoder::OpcodeType::FL:
			FD::fl(decoder);
			break;
		case Decoder::OpcodeType::FS:
			FD::fs(decoder);
			break;
		case Decoder::OpcodeType::FMADD:
			FD::fmadd(decoder);
			break;
		case Decoder::OpcodeType::FMSUB:
			FD::fmsub(decoder);
			break;
		case Decoder::OpcodeType::FNMADD:
			FD::fnmadd(decoder);
			break;
		case Decoder::OpcodeType::FNMSUB:
			FD::fnmsub(decoder);
			break;
		case Decoder::OpcodeType::FOTHER:
			FD::fother(decoder);
			break;
		case Decoder::OpcodeType::ATOMIC:
			A::funct3(decoder);
			break;
		case Decoder::OpcodeType::I64:
			I64::funct3(decoder);
			break;
		case Decoder::OpcodeType::R64:
			R64::funct3(decoder);
			break;
		case Decoder::OpcodeType::AUIPC:
			O::auipc(decoder);
			break;
		case Decoder::OpcodeType::LUI:
			O::lui(decoder);
			break;
		case Decoder::OpcodeType::JAL:
			O::jal(decoder);
			break;
		case Decoder::OpcodeType::JALR:
			O::jalr(decoder);
			break;
		case Decoder::OpcodeType::CSR:
			CSR::funct3(decoder);
			break;
		default:
			cpu->set_exception(
				Exception::ILLEGAL_INSTRUCTION, 
				decoder.insn
			);
			break;
		}
	}

	return decoder.size();
}

}; // namespace Instruction

};
