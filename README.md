# Project Mu UEFI Implementation for Surface Duo Platforms

![Surface Duo Dual Screen Windows](https://user-images.githubusercontent.com/3755345/170788230-a42e624a-d2ed-4070-b289-a9b34774bcd0.png)

## Description

This repository hosts the code and underlying work behind the Surface Duo Windows UEFI firmware "bootstrapper" for Surface Duo 1 and Surface Duo 2 devices.

## Compatibility

| Device Name   | Device Generation/Year | Codenames/Internal Names | UEFI Port Status | Windows Bootability Status |
|---------------|------------------------|--------------------------|------------------|----------------------------|
| Surface Duo   | First generation 2020  | Epsilon, OEMB1, OEMA0 (Bogus?), oema0 oema0 b1 (Bogus?), Andromeda (Bogus?) | ✅ | ✅ |
| Surface Duo 2 | Second generation 2021 | Zeta, OEMC1, oemc1 sf c1, Andromeda (Bogus?) | ✅ | ⚠️* |

*Only a single core will work in Windows currently with the current ACPI tables.

It is not going to work on any other device even if they use the same SoC as is. You may risk breaking your other device if you even try it. It is only for Surface Duo devices, and there is no interest in adding support for other devices in this repository.

## Build

### Minimum System Requirements

- At least 2 cores x86_64 processor running at 2Ghz or higher implementing the X86 ISA with 64 bit AMD extensions (AMD64) (Currently, building on any other ISA is not supported. In other words, do. not. build. this. on. a. phone. running. android. please.)
- SSD
- A linux environment capable of running below tool stack:
  - Bash
  - Python 3.10 or higher (python3.10, python3.10-venv, python3.10-pip)
  - mono-devel
  - git-core, git
  - build-essential
  - PowerShell Core 7
  - clang38 (or higher), llvm, ggc-aarch64-linux-gnu
- Exported CLANG38_BIN environment variable pointing to LLVM 10 binary folder
- Exported CLANG38_AARCH64_PREFIX variable equalling to aarch64-linux-gnu-

### Build Instructions

- Clone this repository to a reasonable location on your disk (There is absolutely no need to initialize submodules, stuart will do it for you later on)
- Run the following commands in order, with 0 typo, and without copy pasting all of them blindly all at once:

```
# Setup environment
sudo ./setup_env.sh

# Stamp
./build_releaseinfo.ps1

# Build UEFI
pip install --upgrade -r pip-requirements.txt
./build_uefi_epsilon.sh
./build_uefi_zeta.sh

# Generate ELF image
./build_bootshim.sh
./build_boot_images.sh
```

## Acknowledgements

- [EFIDroid Project](http://efidroid.org)
- Andrei Warkentin and his [RaspberryPiPkg](https://github.com/andreiw/RaspberryPiPkg)
- Sarah Purohit
- [Googulator](https://github.com/Googulator/)
- [Ben (Bingxing) Wang](https://github.com/imbushuo/)
- [Renegade Project](https://github.com/edk2-porting/)

## License

All code except drivers in `GPLDriver` directory are licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
