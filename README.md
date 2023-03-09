# [Project Mu](https://microsoft.github.io/mu/) UEFI Implementation for Surface Duo Platforms

![Surface Duo Dual Screen Windows](https://user-images.githubusercontent.com/3755345/197420866-d3bb0534-c848-4cc2-a242-04dae48b0f6e.png)

## For users

You can download the latest UEFI build by clicking [here](https://github.com/WOA-Project/SurfaceDuoPkg/releases).

[![Build Status (Visual Studio Team Services)](https://gus33000.visualstudio.com/SurfaceDuoPkg/_apis/build/status/SurfaceDuoPkg%20CI?branchName=main)](https://gus33000.visualstudio.com/SurfaceDuoPkg/_build/latest?definitionId=1&branchName=main)

## What's this?

This package demonstrates an AArch64 UEFI implementation for hacked Surface Duo 1 and Surface Duo 2 devices. Currently it is able to boot Windows 10 ARM64 as well as Windows 11 ARM64. Please be aware that SM8350 devices have limited support.

## Support Status

Applicable to all supported targets unless noted.

- Low-speed I/O: I2C, SPI, GPIO, SPMI and Pinmux (TLMM).
- Power Management: PMIC and Resource Power Manager (RPM).
- High-speed I/O for firmware and HLOS: UFS 3.1
- Peripherals: Touchscreen (HID SPI), side-band buttons (TLMM GPIO and PMIC GPIO)
- Display FrameBuffer

## What can you do?

Please see https://github.com/WOA-Project/SurfaceDuo-Guides for some tutorials.

## Compatibility

| Device Name   | Device Generation/Year | Codenames/Internal Names | UEFI Port Status | Windows Bootability Status |
|---------------|------------------------|--------------------------|------------------|----------------------------|
| Surface Duo   | First generation 2020  | Epsilon, OEMB1, OEMA0 (Bogus?), oema0 oema0 b1 (Bogus?), Andromeda (Bogus?) | ✅ | ✅ |
| Surface Duo 2 | Second generation 2021 | Zeta, OEMC1, oemc1 sf c1, Andromeda (Bogus?) | ✅ | ⚠️* |

*Only a single core will work in Windows currently with the current ACPI tables on Surface Duo 2.

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
  - clang38 (or higher), llvm, gcc-aarch64-linux-gnu
- Exported CLANG38_BIN environment variable pointing to LLVM 10 binary folder
- Exported CLANG38_AARCH64_PREFIX variable equalling to aarch64-linux-gnu-

### Build Instructions

- Clone this repository to a reasonable location on your disk (There is absolutely no need to initialize submodules, stuart will do it for you later on)
- Run the following commands in order, with 0 typo, and without copy pasting all of them blindly all at once:

```
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
- Samuel Tulach and his [Rainbow Patcher](https://github.com/SamuelTulach/rainbow)

## License

All code is licensed under BSD 2-Clause.