# Risc-V 64bit Emulator written in c++
### Features
RV64GC instruction set
## Installing
Use built binaries from this github project's releases tab or build using `make`
## Running
Run the executable from command line or use provided scripts to run a demo

`Usage: ./rv64-emu [options]`
`options:`
`-b, --bios              Path to the BIOS file (required)`
`-d, --dtb               Path to the DTB file (required if kernel provided)`
`-k, --kernel           Path to the kernel file`
`-r, --ram_size       Size of RAM to use in MiB (default 64 MiB)`
`-v, --virtual_drive  Path to the virtual disk image`
`-h, --help              This help message`

## Testing
Use `make test` to run riscv ISA tests automatically.
Two FCVTSD tests do not pass because of QNAN/SNAN fault.
This is a known bug and will be addressed in future releases.
This should not have any impact on emulator usability.

## Dependencies
The following libraries are provided as DLL's in `winlib/`.
You should be able to install them on Linux without any issues.
- **libicuuc76**
- **libicudt76**
- **libSDL2**
- **libSDL2_ttf**
