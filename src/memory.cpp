#include <elf.h>
#include <cstdio>
#include <cstring>
#include "emulator.hpp"
#include "errors.hpp"
#include "memory.hpp"
#include "registers.hpp"

using namespace Emulator;

void Memory::load_img(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        error<FAIL>("Failed to open the file");

	Elf64_Ehdr header;
	if (fread(&header, 1, sizeof(header), file) \
		!= sizeof(header)
	)
		error<FAIL>("Failed to read ELF header");

	if (header.e_ident[EI_MAG0] != ELFMAG0 ||
		header.e_ident[EI_MAG1] != ELFMAG1 ||
		header.e_ident[EI_MAG2] != ELFMAG2 ||
		header.e_ident[EI_MAG3] != ELFMAG3
	)
		error<FAIL>("Not a valid ELF file");

	if (header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_machine != EM_RISCV
	)
		error<FAIL>("Not a RISC-V 64bit ELF file");

	if (!(header.e_flags & EF_RISCV_RVC)) {
		error<WARN>("RVC mode not detected");
		RVC = false;
	}

	registers[Registers::PC] = header.e_entry;

	if (fseek(file, header.e_phoff, SEEK_SET))
		error<FAIL>("Failed to seek to program headers");

	for (size_t i = 0; i < header.e_phnum; ++i) {
		Elf64_Phdr pheader;
		if (fread(&pheader, 1, sizeof(pheader), file) \
			!= sizeof(pheader)
		)
			error<FAIL>("Failed to read program entry");

		if (pheader.p_type == 1) {
			if (fseek(file, pheader.p_offset, SEEK_SET))
				error<FAIL>("Failed to seek to segment");

			if (fread(memory + pheader.p_vaddr, 1, 
				pheader.p_filesz, file) != pheader.p_filesz
			)
				error<FAIL>("Failed to read segment data");
			
			if (pheader.p_memsz > pheader.p_filesz)
				memset(memory + pheader.p_vaddr + pheader.p_filesz, \
					   0, pheader.p_memsz - pheader.p_filesz);
		}
	}

    fclose(file);
}

