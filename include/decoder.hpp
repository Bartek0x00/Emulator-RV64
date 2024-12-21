#pragma once

#include <string_view>
#include <cstdint>
#include <ostream>
#include "errors.hpp"

namespace Emulator {	
	class Decoder {
	private:
		enum class OpcodeType : uint64_t {
			COMPRESSED_QUANDRANT0 = 0x00,
			COMPRESSED_QUANDRANT1 = 0x01,
			COMPRESSED_QUANDRANT2 = 0x02,
			
			LD = 0x03,
			
			FENCE = 0x0f,
			
			AUIPC = 0x17,
			LUI = 0x37,
			
			JALR = 0x67,
			JAL = 0x6f,
			
			I = 0x13,
			ST = 0x23,
			R = 0x33,
			B = 0x63,
			
			FL = 0x07,
			FS = 0x27,
			FMADD = 0x43,
			FMSUB = 0x47,
			FNMADD = 0x4b,
			FNMSUB = 0x4f
			FOTHER = 0x53,
	
			ATOMIC = 0x2f,
	
			I64 = 0x1b,
			R64 = 0x3b,
	
			CSR = 0x73
		};
	
		enum class LdType : uint64_t {
			LB = 0x00, 
			LH = 0x01,
			LW = 0x02, 
			LD = 0x03, 
			LBU = 0x04,
			LHU = 0x05, 
			LWU = 0x06
		};
	
		enum class IType : uint64_t {
			ADDI = 0x00,
			SLLI = 0x01,
			SLTI = 0x02,
			SLTIU = 0x03,
			XORI = 0x04,
			SRI = 0x05,
			ORI = 0x06,
			ANDI = 0x07,
	
			SRLI = 0x00,
			SRAI = 0x10
		};
	
		enum class I64Type : uint64_t {
			ADDIW = 0x00,
			SLLIW = 0x01,
			SRIW = 0x05,
	
			SRLIW = 0x00,
			SRAIW = 0x20
		};
	
		enum class StType : uint64_t {
			SB = 0x00,
			SH = 0x01,
			SW = 0x02,
			SD = 0x03
		};
	
		enum class RType : uint64_t {
			ADDMULSUB = 0x00,
			SLLMULH = 0x01,
			SLTMULHSU = 0x02,
			SLTUMULHU = 0x03,
			XORDIV = 0x04,
			SR = 0x05,
			ORREM = 0x06,
			ANDREM = 0x07,

			ADD = 0x00,
			MUL = 0x01,
			SUB = 0x20,

			SLL = 0x00,
			MULH = 0x01,

			SLT = 0x00,
			MULHSU = 0x01,

			SLTU = 0x00,
			MULHU = 0x01,

			XOR = 0x00,
			DIV = 0x01,

			SRL = 0x00,
			DIVU = 0x01,
			SRA = 0x20,

			OR = 0x00,
			REM = 0x01,

			AND = 0x00,
			REMU = 0x01
		};
	
		enum class R64Type : uint64_t {
			ADDSUBW = 0x00,
			SLLW = 0x01,
			DIVW = 0x04,
			SRW = 0x05,
			REMW = 0x06,
			REMUW = 0x07,

			ADDW = 0x00,
			MULW = 0x01,
			SUBW = 0x20,

			SRLW = 0x00,
			DIVUW = 0x01,
			SRAW = 0x20
		};
		
		enum class BType : uint64_t {
			BEQ = 0x00,
			BNE = 0x01,
			BLT = 0x04,
			BGE = 0x05,
			BLTU = 0x06,
			BGEU = 0x07
		};
		
		enum class AType : uint64_t {
			AMOW = 0x02,
			AMOD = 0x03,
	
			ADD = 0x00,
			SWAP = 0x01,
			LR = 0x02,
			SC = 0x03,
			XOR = 0x04,
			OR = 0x08,
			AND = 0x0c,
			MIN = 0x10,
			MAX = 0x14,
			MINU = 0x18,
			MAXU = 0x1c
		};
		
		enum class CSRType : uint64_t {
			ENVIRONMENT = 0x00,
			CSRW = 0x01,
			CSRS = 0x02,
			CSRC = 0x03,
			CSRWI = 0x05,
			CSRSI = 0x06,
			CSRCI = 0x07,

			ECALL = 0x00,
			EBREAK = 0x01,
			RET = 0x02,
			WFI = 0x05,
			OTHER = 0x06,

			ECALL7 = 0x00,
			EBREAK7 = 0x00,
			URET7 = 0x00,
			SRET7 = 0x08,
			MRET7 = 0x18,
			WFI7 = 0x08,
			SFENCEVMA7 = 0x09,
			HFENCEBVMA7 = 0x11,
			HFENCEGVMA7 = 0x51
		};
	
		enum class FdType : uint64_t {
			FLW = 0x02,
			FLD = 0x03,

			FSW = 0x02,
			FSD = 0x03,

			FMADDS = 0x00,
			FMADDD = 0x01,

			FNMADDS = 0x00,
			FNMADDD = 0x01,

			FNMSUBS = 0x00,
			FNMSUBD = 0x01,

			FADDS = 0x00,
			FADDD = 0x01,
	
			FSUBS = 0x04,
			FSUBD = 0x05,
	
			FMULS = 0x08,
			FMULD = 0x09,
	
			FDIVS = 0x0c,
			FDIVD = 0x0d,
	
			FSNGJS = 0x10,
			FSNGJD = 0x11,
	
			FMINMAXS = 0x14,
			FMINMAXD = 0x15,
	
			FCVTSD = 0x20,
			FCVTDS = 0x21,
			FSQRTS = 0x2c,
			FSQRTD = 0x2d,
	
			FCS = 0x50,
			FCD = 0x51,
	
			FCVTS = 0x60,
			FCVTD = 0x61,
	
			FCVTSW = 0x68,
			FCVTDW = 0x69,
	
			FMVXW = 0x70,
			FMVXD = 0x71,
	
			FMVWX = 0x78,
			FMVDX = 0x79,

			FSGNJ = 0x00,
			FSGNJN = 0x01,
			FSGNJX = 0x02,

			MIN = 0x00,
			MAX = 0x01,

			FLE = 0x00,
			FLT = 0x01,
			FEQ = 0x02,

			FCVT0 = 0x00,
			FCVT1 = 0x01,
			FCVT2 = 0x02,
			FCVT3 = 0x03,

			FMV = 0x00,
			FCLASS = 0x01
		};

		enum class Q0 : uint64_t {
			ADDI4SPN = 0x00,
        	FLD = 0x01,
    		LW = 0x02,
        	LD = 0x03,
        	RESERVED = 0x04,
        	FSD = 0x05,
        	SW = 0x06,
        	SD = 0x07,
		};

		enum class Q1 : uint64_t {
			ADDI = 0x00,
        	ADDIW = 0x01,
        	LI = 0x02,

        	OP03 = 0x03,
        	OP04 = 0x04,

        	J = 0x05,
        	BEQZ = 0x06,
        	BNEZ = 0x07,

			NOP = 0x00,
        	ADDI16SP = 0x02,
        	LUI = 0x03

			SRLI = 0x00,
            SRAI = 0x01,
            ANDI = 0x02,
		};

		enum class Q2 : uint64_t {
			SLLI = 0x00,
    		FLDSP = 0x01,
        	LWSP = 0x02,
        	LDSP = 0x03,

        	OP04 = 0x04,

        	FSDSP = 0x05,
        	SWSP = 0x06,
        	SDSP = 0x07,
		};
	
		const uint32_t insn;

	public:
		inline Decoder(uint32_t _insn) : insn(_insn) {};

		inline uint64_t opcode(void) const 
		{
			return insn & 0x7fU;
		}

		inline uint64_t rd(void) const
		{
			return (insn >> 7U) & 0x1fU;
		}

		inline uint64_t rs1(void) const 
		{
			return (insn >> 15U) & 0x1fU;
		}

		inline uint64_t rs2(void) const 
		{
			return (insn >> 20U) & 0x1fU;
		}
		
		inline uint64_t rs3(void) const 
		{
			return (insn & 0xf8000000U) >> 27U;
		}
		
		inline uint64_t funct2(void) const 
		{
			return (insn & 0x03000000U) >> 25U;
		}
		
		inline uint64_t funct3(void) const 
		{
			return (insn >> 12U) & 0x7U;
		}
		
		inline uint64_t funct5(void) const 
		{
			return (funct7() & 0x7cU) >> 2U;
		}

		inline uint64_t funct7(void) const 
		{
			return (insn >> 25U) 0x7fU;
		}
		
		inline uint64_t imm_i(void) const 
		{
			return static_cast<int64_t>(insn & 0xfff00000U) >> 20U;
		}

		inline uint64_t imm_s(void) const
		{
			return (static_cast<int64_t>(insn & 0xfe000000U) >> 20U) | ((insin >> 7) & 0x1fU);
		}

		inline uint64_t imm_b(void) const
		{
			return (static_cast<int64_t>(insn & 0x80000000U) >> 19U) |
				((insn & 0x80U) << 4U) | 
				((insn >> 20U) & 0x7e0U) |
				((insn >> 7U) & 0x1eU);  
		}

		inline uint64_t imm_u(void) const
		{
			return insn & 0xfffff999U;
		}

		inline uint64_t imm_j(void) const
		{
			return (static_cast<int64_t>(insn & 0x80000000U) >> 11U) |
				(insn & 0xff000U) |
				((insn >> 9U) & 0x800U) |
				((insn >> 20U) & 0x7feU);
		}

		inline uint64_t fl_off(void) const
		{
			return static_cast<int64_t>(insn) >> 20;
		}

		inline uint64_t fs_off(void) const
		{
			return (dl_off() & 0xfe0U) | ((insn >> 7U) & 0x1fU);
		}

		inline uint64_t shamt(void) const
		{
			return imm_i() & 0x3fU;
		}

		inline uint64_t csr(void) const
		{
			return (insn & 0xfff00000U) >> 20U;
		}

		inline uint64_t size(void) const
		{
			switch(reinterpret_cast<OpcodeType>(opcode_c())) {
			case COMPRESSED_QUANDRANT0:
			case COMPRESSED_QUANDRANT1:
			case COMPRESSED_QUANDRANT2:
				return 2;
			default:
				return 4;
			}
		}

		inline uint64_t opcode_c(void) const
		{
			return insn & 0x03U;
		}

		inline uint64_t rd_c(void) const
		{
			return ((insn >> 2U) & 0x07U) + 8U;
		}

		inline uint64_t rs1_c(void) const
		{
			return ((insn >> 7U) & 0x07U) + 8U;
		}

		inline uint64_t rs2_c(void) const
		{
			return ((insn >> 2U) & 0x07) + 8u;
		}

		inline uint64_t shamt_c(void) const
		{
			return ((insn >> 7U) & 0x20U) | ((insn >> 2U) & 0x1fU);
		}

		inline uint64_t funct2_c(void) const
		{
			return (insn >> 10U) & 0x3U;
		}

		inline uint64_t funct3_c(void) const
		{
			return (insn >> 13U) & 0x07U;
		}

		inline uint64_t rounding_mode(void) const
		{
			return (insn >> 12U) & 0x3U;
		}

		inline void dump(void) const
		{
			string_view name;

			switch (static_cast<OpcodeType>(opcode())) {
			case OpcodeType::LD: 		name = "LOAD"; 		break;
			case OpcodeType::FENCE: 	name = "FENCE"; 	break;
			case OpcodeType::AUIPC: 	name = "AUIPC"; 	break;
			case OpcodeType::LUI: 		name = "LUI"; 		break;
			case OpcodeType::JALR: 		name = "JALR"; 		break;
			case OpcodeType::JAL: 		name = "JAL"; 		break;
			case OpcodeType::I: 		name = "IMM"; 		break;
			case OpcodeType::ST: 		name = "STORE"; 	break;
			case OpcodeType::R: 		name = "REGISTER"; 	break;
			case OpcodeType::B: 		name = "BRANCH"; 	break;
			case OpcodeType::FL: 		name = "FLT"; 		break;
			case OpcodeType::FS: 		name = "FLTS"; 		break;
			case OpcodeType::FMADD: 	name = "FMADD"; 	break;
			case OpcodeType::FMSUB: 	name = "FMSUB"; 	break;
			case OpcodeType::FNMADD:	name = "FNMADD"; 	break;
			case OpcodeType::FNMSUB:	name = "FNMSUB";	break;
			case OpcodeType::FOTHER:	name = "FOTHER"; 	break;
			case OpcodeType::I64: 		name = "I64"; 		break;
			case OpcodeType::R64: 		name = "R64"; 		break;
			case OpcodeType::CSR: 		name = "CSR"; 		break;
			case OpcodeType::A:			name = "ATOMIC"; 	break;
			default: 					name = "UNKNOWN" 	break;
			}

			if (size() == 2)
				name = "COMPRESSED"

			error<INFO>(
				"################################\n"
				"# Component: Decoder           #\n"
				"################################"
				"\n# insn: ", insn,
				"\n# size: ", size(),
				"\n# opcode: ", name, " (", opcode(), ") ",
				"\n# opcode_c: ", " (", opcode_c(), ") ",
				"\n# src r1: ", rs1(),
				"\n# src r2: ", rs2(),
				"\n# src r3: ", rs3(),
				"\n# dst r: ", rd(),
				"\n# imm_i: ", imm_i(),
				"\n# imm_s: ", imm_s(),
				"\n# imm_b: ", imm_b(),
				"\n# imm_u: ", imm_u(),
				"\n# imm_j: ", imm_j(),
				"\n# funct2: ", funct2(),
				"\n# funct3: ", funct3(),
				"\n# funct5: ", funct5(),
				"\n# funct7: ", funct7(),
				"\n# funct2_c: ", funct2_c(),
				"\n# funct3_c: ", funct3_c(),
				"\n# shamt: ", shamt(),
				"\n# csr: ", csr(),
				"\n################################"
			);
		}
	};
};
