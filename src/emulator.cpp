#include <filesystem>
#include <getopt.h>
#include "emulator.hpp"
#include "settings.hpp"

using namespace Emulator;

static bool patch_dtb_ram_size(
	std::vector<uint8_t>& dtb_data, 
	uint32_t ram_size)
{
	static constexpr std::array<uint8_t, 4> pattern = {
		0x0b, 0xad, 0xc0, 0xde
	};

	std::vector<uint8_t>::iterator it = std::search(
		dtb_data.begin(), dtb_data.end(),
		pattern.begin(), pattern.end()
	);

	if (it == dtb_data.end())
		return false;
	
	uint32_t ram_size_be = (
		((ram_size & 0xff000000U) >> 24U) |
		((ram_size & 0x00ff0000U) >> 8U) |
		((ram_size & 0x0000ff00U) << 8U) |
		((ram_size & 0x000000ffU) << 24U)
	);

	std::memcpy(&*it, &ram_size_be, 4);

	return true;
}

Emulator::Emulator(int argc, char *argv[])
{
	string_view bios_p = "";
	string_view dtb_p = "";
	string_view kernel_p = "";
	string_view virt_drive_p = "";

	uint64_t ram_size = RAM_SIZE;

	static constexpr option long_options[] = {
		{"bios", required_argument, nullptr, 'b'},
		{"dtb", required_argument, nullptr, 'd'},
		{"kernel", required_argument, nullptr, 'k'},
		{"ram_size", required_argument, nullptr, 'r'},
		{"virtual_drive", required_argument, nullptr, 'v'},
		{}
	};
	
	int opt = 0;
	int opt_idx = 0;

	while ((opt = getopt_long(argc, argv, "b:d:k:r:v:", 
							long_options, &opt_idx)
	) != -1) {
		switch (opt) {
		case 'b':
			bios_p = optarg;
			break;
		case 'd':
			dtb_p = optarg;
			break;
		case 'k':
			kernel_p = optarg;
			break;
		case 'r':
			ram_size = size<MIB>(atoi(optarg));
			break;
		case 'v':
			virt_drive_p = optarg;
			break;
		default:
			error<FAIL>(
				"Usage: ", argv[0], " [options]\n"
				"options: \n"
				"  -b, --bios 			Path to the BIOS file (required)\n"
				"  -d, --dtb  			Path to the DTB file (required if kernel provided)\n"
				"  -k, --kernel 		Path to the kernel file\n"
				"  -r, --ram_size 		Size of RAM to use in MiB (default 64 MiB)\n"
				"  -v, --virtual_drive 	Path to the virtual disk image\n"
			);
			break;
		}
		
		if (!bios_p)
			error<FAIL>("bios path must be provided!\n");
		
		if (kernel_p && !dtb_p)
			error<FAIL>("dtb path must be provided when kernel is used!\n");

		if (!std::filesystem::exists(bios_p))
			error<FAIL>("bios path invalid\n");

		uint64_t ram_size_dtb = ram_size;

		if (dtb_p)
			ram_size_dtb += size<MIB>(2);
		
		cpu.mode = Cpu::Mode::MACHINE;
		cpu.pc = DRAM_BASE;
		cpu.int_regs[IRegs::sp] = DRAM_BASE + ram_size_dtb;
		
		bus.add<Dram>(
			DRAM_BASE,
			ram_size_dtb,
			std::move(load_file(bios_p))
		);

		bus.add<Plic>();
		bus.add<Clint>();
		bus.add<Gpu>(960, 540);
		
		if (virt_drive_p) {
			if (!std::filesystem::exists(virt_drive_p))
				error<FAIL>("virt_drive path invalid\n");

			bus.add<Virtio>(
				std::move(load_file(virt_drive_p))
			);
		}

		bus.add<Syscon>();

		if (dtb_p) {
			if (!std::filesystem::exists(dtb_p))
				error<FAIL>("dtb path invalid\n");

			std::vector<uint8_t> dtb = load_file(dtb_p);
			cpu.int_regs[IRegs::a1] = DRAM_BASE + ram_size;
		
			if (!patch_dtb_ram_size(dtb, ram_size))
				error<WARN>(
					"could not find device tree binary\n"
					"memory size magic value (0x0badc0de),\n"
					"make sure that the amount of memory\n"
					"that the emulator allocates is equal\n"
					"or greater than specified in the dtb\n"
				);

			bus["DRAM"].copy(dtb, ram_size);
		}

		if (kernel_p) {
			if (!std::filesystem::exists(kernel_p)
				error<FAIL>("kernel path invalid\n);

			std::vector<uint8_t> kernel = load_file(kernel_p);
			bus["DRAM"].copy(kernel, KERNEL_OFFSET);
		}

		cpu.run();
	}
}
