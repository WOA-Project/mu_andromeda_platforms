#!/bin/bash

export CLANG38_BIN=/usr/lib/llvm-14/bin/ && export CLANG38_AARCH64_PREFIX=aarch64-linux-gnu-

python3 ./Platforms/SurfaceDuo2Pkg/PlatformBuild.py TARGET=RELEASE SECURE_BOOT_ENABLE=FALSE

cd BootShim
make UEFI_BASE=0x9FC41000 UEFI_SIZE=0x002BF000
mv ./BootShim.bin ./BootShim.Zeta.bin
mv ./BootShim.elf ./BootShim.Zeta.elf
cd ..

cat ./BootShim/BootShim.Zeta.bin ./Build/SurfaceDuo2Pkg/RELEASE_CLANG38/FV/SM8350_EFI.fd > ./ImageResources/Zeta/bootpayload.bin
cat ./ImageResources/Zeta/patchedkernel ./Build/SurfaceDuo2Pkg/RELEASE_CLANG38/FV/SM8350_EFI.fd > ./ImageResources/Zeta/dualbootbootpayload.bin

python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/bootpayload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/uefi.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01

python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/dualbootbootpayload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/uefi_dualboot.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01
