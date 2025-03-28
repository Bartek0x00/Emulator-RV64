#include <iostream>
#include <filesystem>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "bus.hpp"
#include "dram.hpp"
#include "plic.hpp"
#include "clint.hpp"
#include "gpu.hpp"
#include "syscon.hpp"
#include "virtio.hpp"
#include "emulator.hpp"

using namespace Emulator;

static bool is_debugger_present(void)
{
	char buf[4096];

	const int status_fd = open("/proc/self/status", O_RDONLY);
	if (status_fd == -1)
		return false;
	
	const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
	close(status_fd);

	if (num_read <= 0)
		return false;

	buf[num_read] = '\0';
	constexpr char tracer_pid_str[] = "TracerPid:";
	const char *tracer_pid_ptr = strstr(buf, tracer_pid_str);
	if (!tracer_pid_ptr)
		return false;

	for (const char *ch_ptr = tracer_pid_ptr + sizeof(tracer_pid_str) - 1;
			ch_ptr <= buf + num_read; ++ch_ptr)
	{
		if (isspace(*ch_ptr))
			continue;
		else
			return (
				isdigit(*ch_ptr) != 0 &&
				*ch_ptr != '0'
			);
	}

	return false;
}

static constexpr uint64_t TO_HOST_OFF = 0x1000U;
static constexpr uint64_t TO_HOST_OFF_C = 0x3000U;
static constexpr uint64_t RAM_OFF = 0x80000000U;

namespace fs = std::filesystem;

static bool test_bin(const fs::directory_entry& bin_path)
{
	cpu = std::make_unique<Cpu>();
	mmu = std::make_unique<Mmu>();
	bus = std::make_unique<Bus>();

	cpu->int_regs[IRegs::sp] = RAM_OFF + BYTE_SIZE<KIB>(64);

	bus->add<Dram, DeviceName::DRAM>(
		RAM_OFF,
		BYTE_SIZE<KIB>(64),
		load_file(bin_path.path().c_str())
	);
	
	bus->add<Plic, DeviceName::PLIC>();
	bus->add<Clint, DeviceName::CLINT>();

	uint64_t start = get_milliseconds();
	uint64_t a0 = -1;

	bool timeout = false;
	bool failed_on_exception = false;
	bool debugger_present = is_debugger_present();

	while (!timeout) {
		cpu->iterate();

		Dram *dram = static_cast<Dram*>(
			bus->get(DeviceName::DRAM)
		);
		if (dram)
			if (dram->load(TO_HOST_OFF + RAM_OFF, 64) ||
				dram->load(TO_HOST_OFF_C + RAM_OFF, 64))
			{
				a0 = cpu->int_regs[IRegs::a0];
				break;
			}

		if (cpu->exception.current != Exception::NONE &&
			cpu->csr_regs.load(CRegs::Address::MTVEC) == 0 &&
			cpu->csr_regs.load(CRegs::Address::STVEC) == 0)
		{
			failed_on_exception = true;
			break;
		} 
		else
			cpu->clear_exception();

		if (!debugger_present)
			timeout = get_milliseconds() - start > 5000;
	}

	if (a0 == 0)
		return true;

	std::cout << RED << "Test failed with pc = " << cpu->pc << CLEAR << '\n';

	if (!timeout) {
		if (failed_on_exception)
			std::cout << RED << "Exception: " << cpu->exception.get_name() << CLEAR << '\n';
		else
			std::cout << RED << "Execution failed with a0 != 0";
			std::cout << "\n a0 = " << a0;
			std::cout << "\n gp = " << cpu->int_regs[IRegs::gp];
			std::cout << CLEAR << '\n';
	} else
		std::cout << RED << "Timeout!\n" << CLEAR;
	
	return false;
}

int main(void)
{
	for (const fs::directory_entry& entry : fs::directory_iterator("test/riscv-isa")) {
		if (!fs::is_directory(entry.status()))
			continue;
		
		bool has_failed = false;
		for (const fs::directory_entry& sub_entry : fs::directory_iterator(entry.path())) {
			if (!fs::is_regular_file(sub_entry.status()) ||
				(sub_entry.path().extension() != ".bin"))
			{
				continue;
			}
			
			std::cerr << sub_entry.path().string() << '\n';

			if (test_bin(sub_entry))
				std::cout << GREEN << "[PASS] " << CLEAR;
			else {
				std::cout << RED << "[FAIL] " << CLEAR;
				has_failed = true;
			}

			std::cout << sub_entry.path().string() << '\n';
		}
		
		std::cout << '\n';
		if (has_failed) {
			std::cout << RED << "################################\n"
								"# [FAILED] " << entry.path().string() << "\n" <<
								"################################\n\n\n" << CLEAR;	
		} else {
			std::cout << GREEN << "################################\n"
								  "# [PASSED] " << entry.path().string() << "\n" <<
								  "################################\n\n\n" << CLEAR;
		}
	}

	return 0;
}
