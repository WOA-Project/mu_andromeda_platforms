#!/bin/bash

export CLANG38_BIN=/usr/lib/llvm-14/bin/ && export CLANG38_AARCH64_PREFIX=aarch64-linux-gnu-

python3 ./Platforms/SurfaceDuo1Pkg/PlatformBuild.py TARGET=RELEASE

cd BootShim
make UEFI_BASE=0x9FC00000 UEFI_SIZE=0x00300000
cd ..

cat ./BootShim/BootShim.bin ./Build/SurfaceDuo1Pkg/RELEASE_CLANG38/FV/SM8150_EFI.fd > ./ImageResources/Epsilon/bootpayload.bin
cat ./ImageResources/Epsilon/patchedkernel ./Build/SurfaceDuo1Pkg/RELEASE_CLANG38/FV/SM8150_EFI.fd > ./ImageResources/Epsilon/dualbootbootpayload.bin

python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Epsilon/bootpayload.bin \
  --ramdisk ./ImageResources/Epsilon/ramdisk \
  -o ./ImageResources/Epsilon/uefi.img \
  --pagesize 4096 \
  --header_version 2 \
  --cmdline "console=ttyMSM0,115200n8 earlycon=msm_geni_serial,0xa90000 androidboot.hardware=surfaceduo androidboot.hardware.platform=qcom androidboot.console=ttyMSM0 androidboot.memcg=1 lpm_levels.sleep_disabled=1 video=vfb:640x400,bpp=32,memsize=3072000 msm_rtb.filter=0x237 service_locator.enable=1 swiotlb=2048 loop.max_part=7 androidboot.usbcontroller=a600000.dwc3 kpti=off buildvariant=user" \
  --dtb ./ImageResources/Epsilon/dtb \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01 \
  --second_offset 0xf00000

python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Epsilon/dualbootbootpayload.bin \
  --ramdisk ./ImageResources/Epsilon/ramdisk \
  -o ./ImageResources/Epsilon/uefi_dualboot.img \
  --pagesize 4096 \
  --header_version 2 \
  --cmdline "console=ttyMSM0,115200n8 earlycon=msm_geni_serial,0xa90000 androidboot.hardware=surfaceduo androidboot.hardware.platform=qcom androidboot.console=ttyMSM0 androidboot.memcg=1 lpm_levels.sleep_disabled=1 video=vfb:640x400,bpp=32,memsize=3072000 msm_rtb.filter=0x237 service_locator.enable=1 swiotlb=2048 loop.max_part=7 androidboot.usbcontroller=a600000.dwc3 kpti=off buildvariant=user" \
  --dtb ./ImageResources/Epsilon/dtb \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01 \
  --second_offset 0xf00000