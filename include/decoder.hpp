#pragma once

#include <cstdint>
#include <ostream>
	
namespace Emulator {	
	class Decoder {
	private:
		enum class OpcodeType : uint64_t {
			COMPRESSED_QUANDRANT0 = 0x00,
			COMPRESSED_QUANDRANT1 = 0x01,
			COMPRESSED_QUANDRANT2 = 0x02,
			
			LOAD = 0x03,
			
			FENCE = 0x0f,
			
			AUIPC = 0x17,
			LUI = 0x37,
			
			JALR = 0x67,
			JAL = 0x6f,
			
			I = 0x13,
			S = 0x23,
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
	
		enum class LoadType : uint64_t {
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
	
		enum class SType : uint64_t {
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
		};
	
		enum class AddMulSub : uint64_t {
			ADD = 0x00,
			MUL = 0x01,
			SUB = 0x20
		};
	
		enum class SllMulh : uint64_t {
			SLL = 0x00,
			MULH = 0x01
		};
	
		enum class SltMulhsu : uint64_t {
			SLT = 0x00,
			MULHSU = 0x01
		};
		
		enum class SltuMulhu : uint64_t {
			SLTU = 0x00,
			MULHU = 0x01
		};
	
		enum class XorDiv : uint64_t {
			XOR = 0x00,
			DIV = 0x01
		};
	
		enum class Sr : uint64_t {
			SRL = 0x00,
			DIVU = 0x01,
			SRA = 0x20
		};
		
		enum class OrRem : uint64_t {
			OR = 0x00,
			REM = 0x01
		};
	
		enum class AndRemu : uint64_t {
			AND = 0x00,
			REMU = 0x01
		};
	
		enum class R64Type : uint64_t {
			ADDSUBW = 0x00,
			SLLW = 0x01,
			DIVW = 0x04,
			SRW = 0x05,
			REMW = 0x06,
			REMUW = 0x07
		};
	
		enum class AddSubW : uint64_t {
			ADDW = 0x00,
			MULW = 0x01,
			SUBW = 0x20
		};
	
		enum class SrW : uint64_t {
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
		
		enum class CType : uint64_t {
			ENVIRONMENT = 0x00,
			CSRW = 0x01,
			CSRS = 0x02,
			CSRC = 0x03,
			CSRWI = 0x05,
			CSRSI = 0x06,
			CSRCI = 0x07
		};
	
		enum class EnvironmentImm : uint64_t {
			ECALL = 0x00,
			EBREAK = 0x01,
			RET = 0x02,
			WFI = 0x05,
			OTHER
		};
	
		enum class Environment : uint64_t {
			ECALL = 0x00,
			EBREAK = 0x00,
			URET = 0x00,
			SRET = 0x08,
			MRET = 0x18,
			WFI = 0x08,
			SFENCEVMA = 0x09,
			HFENCEBVMA = 0x11,
			HFENCEGVMA = 0x51
		};
	
		enum class Fl : uint64_t {
			FLW = 0x02,
			FLD = 0x03
		};
	
		enum class Fs : uint64_t {
			FSW = 0x02,
			FSD = 0x03
		};
	
		enum class FmAdd : uint64_t {
			FMADDS = 0x00,
			FMADDD = 0x01
		};
	
		enum class FnmAdd : uint64_t {
			FNMADDS = 0x00,
			FNMADDD = 0x01
		};
	
		enum class FnmSub : uint64_t {
			FNMSUBS = 0x00,
			FNMSUBD = 0x01
		};
	
		enum class Fother : uint64_t {
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
			FMVDX = 0x79
		};
	
		enum class Fsgnj : uint64_t {
			FSGNJ = 0x00,
			FSGNJN = 0x01,
			FSGNJX = 0x02
		};
	
		enum class FminMax : uint64_t {
			MIN = 0x00,
			MAX = 0x01
		};
	
		enum class Fc : uint64_t {
			FLE = 0x00,
			FLT = 0x01,
			FEQ = 0x02
		};
	
		enum class Fcvt : uint64_t {
			FCVT0 = 0x00,
			FCVT1 = 0x01,
			FCVT2 = 0x02,
			FCVT3 = 0x03
		};
	
		enum class Fmv : uint64_t {
			FMV = 0x00,
			FCLASS = 0x01
		}
	
		const uint32_t insn;

	public:
		inline Decoder(uint32_t _insn) : insn(_insn) {};
		
		inline uint64_t fl_off(void) const {return }

		inline uint64_t opcode(void) const {return insn & 0x7fU;}
		inline uint64_t rd(void) const {return ((insn >> 7U) & 0x1fU);}
		inline uint64_t rs1(void) const {return ((insn >> 15U) & 0x1fU);}
		inline uint64_t rs2(void) const {return ((insn >> 20U) & 0x1fU);}
		inline uint64_t rs3(void) const {return ((insn & 0xf8000000U) >> 27U);}
		inline uint64_t funct2(void) const {return ((insn & 0x03000000U) >> 25U);}
		inline uint64_t funct3(void) const {return ((insn >> 12U) & 0x7U);}
		inline uint64_t funct5(void) const {return (funct7() & 0x7cU) >> 2U;}
		inline uint64_t funct7(void) const {return (insn >> 25U) 0x7fU;}
		inline uin
	};
};
