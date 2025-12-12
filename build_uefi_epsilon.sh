#!/bin/bash

# Andromeda855 UEFI as Boot
cat ./BootShim/BootShim.Epsilon.bin ./ImageResources/Epsilon/SM8150_EFI.fd > ./ImageResources/Epsilon/uefi.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Epsilon/uefi.payload.bin \
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

# Andromeda855 UEFI as Boot (SecureBoot Disabled)
cat ./BootShim/BootShim.Epsilon.bin ./ImageResources/Epsilon/SM8150_EFI_NOSB.fd > ./ImageResources/Epsilon/uefi_nosb.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Epsilon/uefi_nosb.payload.bin \
  --ramdisk ./ImageResources/Epsilon/ramdisk \
  -o ./ImageResources/Epsilon/uefi_nosb.img \
  --pagesize 4096 \
  --header_version 2 \
  --cmdline "console=ttyMSM0,115200n8 earlycon=msm_geni_serial,0xa90000 androidboot.hardware=surfaceduo androidboot.hardware.platform=qcom androidboot.console=ttyMSM0 androidboot.memcg=1 lpm_levels.sleep_disabled=1 video=vfb:640x400,bpp=32,memsize=3072000 msm_rtb.filter=0x237 service_locator.enable=1 swiotlb=2048 loop.max_part=7 androidboot.usbcontroller=a600000.dwc3 kpti=off buildvariant=user" \
  --dtb ./ImageResources/Epsilon/dtb \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01 \
  --second_offset 0xf00000
