#pragma once

#include <array>
#include <cstdint>
#include "errors.hpp"

namespace Emulator {

    class IRegs {
	private:
		enum IRegs : uint64_t {
			x0 = 0, x1,  
			x2,  x3,	 
			x4,  x5,  
			x6,  x7,  
			x8,  x9,  
			x10, x11, 
			x12, x13, 
			x14, x15, 
			x16, x17, 
			x18, x19, 
			x20, x21, 
			x22, x23, 
			x24, x25, 
			x26, x27, 
			x28, x29, 
			x30, x31,

			zero = x0,
			ra = x1,  sp = x2,
			gp = x3,  tp = x4,
			t0 = x5,  t1 = x6,
			t2 = x7,  s0 = x8
			fp = x8,  s1 = x9,  
			a0 = x10, a1 = x11, 
			a2 = x12, a3 = x13, 
			a4 = x14, a5 = x15, 
			a6 = x16, a7 = x17, 
			s2 = x18, s3 = x19, 
			s4 = x20, s5 = x21, 
			s6 = x22, s7 = x23, 
			s8 = x24, s9 = x25, 
			s10 = x26, s11 = x27, 
			t3 = x28, t4 = x29, 
			t5 = x30, t6 = x31
		};

    	std::array<uint64_t, 32> regs;

    public:
    	explicit constexpr inline IRegs(void) : regs{};

		inline uint64_t& operator[](IRegs index)
		{
			if (index >= 32)
				error<FAIL>(
					"Integer Register access out of bounds: ", 
					index
				);
			
			return regs[index];
		}
	};

	class FRegs {
	private:
		enum FRegs : uint64_t {
			ft0 = 0, ft1,
			ft2, ft3,
			ft4, ft5,
			ft6, ft7,
			fs0, fs1,
			fa0, fa1,
			fa2, fa3,
			fa4, fa5,
			fa6, fa7,
			fs2, fs3,
			fs4, fs5,
			fs6, fs7,
			fs8, fs9,
			fs10, fs11,
			ft8, ft9,
			ft10, ft11
		};
		
		std::array<double, 32> regs;

	public:
		explicit constexpr inline FRegs(void) : regs{};

		inline double& operator[](FRegs index)
		{
			if (index >= 32)
				error<FAIL>(
					"Float Register access out of bounds: ",
					index
				);

			return regs[index];
		}
	};

	class CRegs {
	private:
		enum class Address : uint64_t {
			USTATUS = 0x0,

        	FFLAGS = 0x1,
        	FRM = 0x2,
        	FCSR = 0x3,

        	UVEC = 0x5,
        	UEPC = 0x41,
        	UCAUSE = 0x42,
        	UTVAL = 0x43,

        	SSTATUS = 0x100,
        	SEDELEG = 0x102,
        	SIDELEG = 0x103,
        	SIE = 0x104,
        	STVEC = 0x105,

        	SSCRATCH = 0x140,
        	SEPC = 0x141,
        	SCAUSE = 0x142,
        	STVAL = 0x143,
        	SIP = 0x144,

        	SATP = 0x180,

        	MSTATUS = 0x300,
        	MISA = 0x301,
        	MEDELEG = 0x302,
        	MIDELEG = 0x303,
        	MIE = 0x304,
        	MTVEC = 0x305,
        	MCOUNTEREN = 0x306,

        	MSCRATCH = 0x340,
        	MEPC = 0x341,
        	MCAUSE = 0x342,
        	MTVAL = 0x343,
        	MIP = 0x344,

        	CYCLE = 0xc00,
        	TIME = 0xc01,
        	TIMEMS = 0xc10,

        	TDATA1 = 0x7a1,

        	MVENDORID = 0xf11,
        	MARCHID = 0xf12,
        	MIMPID = 0xf13,
        	MHARTID = 0xf14,
		};

		enum class Mask : uint64_t {
			SIE = 0x2,
        	SPIE = 0x20,
        	UBE = 0x40,
        	SPP = 0x100,
        	FS = 0x6000,
        	XS = 0x18000,
        	SUM = 0x40000,
        	MXR = 0x80000,
        	UXL = 0x300000000,
        	SD = 1ULL << 63,

			SSTATUS = \
				SIE | SPIE | UBE | SPP | \
				FS | XS | SUM | \
				MXR | UXL | SD,

			SIE = 0x1,
			SPIE = 0x5,
			SPP = 0x8,

			MIE = 0x3,
        	MPIE = 0x7,
        	MPP = 0xc,
        	MPRV = 0x11,
        	SUM = 0x12,
        	MXR = 0x13,
        	TVM = 0x14,
        	TSR = 0x16,

			SSIP_BIT = 0x1,
        	MSIP_BIT = 0x3,
        	STIP_BIT = 0x5,
        	MTIP_BIT = 0x7,
        	SEIP_BIT = 0x9,
        	MEIP_BIT = 0xb,

        	SSIP = 1ULL << SSIP_BIT,
        	MSIP = 1ULL << MSIP_BIT,
        	STIP = 1ULL << STIP_BIT,
        	MTIP = 1ULL << MTIP_BIT,
        	SEIP = 1ULL << SEIP_BIT,
        	MEIP = 1ULL << MEIP_BIT,
		};

		enum class Misa : uint64_t {
			A_EXT = 1U << 0U,
        	C_EXT = 1U << 2U,
        	D_EXT = 1U << 3U,
        	RV32E = 1U << 4U,
        	F_EXT = 1U << 5U,
        	HYPERVISOR = 1U << 7U,
        	RV32I_64I_128I = 1U << 8U,
        	M_EXT = 1U << 12U,
        	N_EXT = 1U << 13U,
        	QUAD_EXT = 1U << 16U,
        	SUPERVISOR = 1U << 18U,
        	USER = 1U << 20U,
        	NON_STD_PRESENT = 1U << 22U,

        	XLEN_32 = 1U << 31U,
        	XLEN_64 = 2ULL << 62U,
		};

		enum class FExcept : uint64_t {
			INEXACT = 	0b00001,
        	UNDERFLOW = 0b00010,
        	OVERFLOW = 	0b00100,
        	DIVBYZERO = 0b01000,
        	INVALID = 	0b10000,

        	MASK = INEXACT | UNDERFLOW | \
				OVERFLOW | DIVBYZERO | INVALID,
		};

		enum class FS : uint64_t {
			OFF = 0x00,
			INITIAL = 0x01,
			CLEAN = 0x02,
			DIRTY = 0x03
		};

		std::array<uint64_t, 4096> regs;
	
	public:
		explicit constexpr inline CRegs(void) : regs{};

		inline uint64_t load(uint64_t addr)
		{
			switch (addr) {
    		case Address::SSTATUS:
        		return regs[Address::MSTATUS] & Mask::SSTATUS;
    		case Address::SIE:
        		return regs[Address::MIE] & regs[Address::MIDELEG];
    		case Address::SIP:
        		return regs[Address::MIP] & regs[Address::MIDELEG];
   		 	case Address::MSTATUS:
        		return regs[Address::MSTATUS] | 0x200000000ULL;
    		default:
        		return regs[addr];
    		}
		}

		inline void store(uint64_t addr, uint64_t value)
		{
			switch(addr) {
			case Address::SSTATUS: {
        		uint64_t val = (regs[Address::MSTATUS] & ~Mask::SSTATUS) | (value & Mask::SSTATUS);
        		regs[Address::MSTATUS] = val;
        		break;
    		}
    		case Address::SIE: {
        		uint64_t val =
            	(regs[Address::MIE] & ~regs[Address::MIDELEG]) | (value & regs[Address::MIDELEG]);
        		regs[Address::MIE] = val;
        		break;
    		}
    		case Address::SIP: {
        		uint64_t mask = regs[Address::MIDELEG] & Mask::SSIP;
        		uint64_t val = (regs[Address::MIP] & ~mask) | (value & mask);
       			regs[Address::MIP] = val;
        		break;
    		}
    		default:
        		regs[addr] = value;
				break;
			}
		}
	};
};
