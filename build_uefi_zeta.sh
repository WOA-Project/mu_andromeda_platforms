#!/bin/bash

# Andromeda888 UEFI as Boot
cat ./BootShim/BootShim.Zeta.bin ./ImageResources/Zeta/SM8350_EFI.fd > ./ImageResources/Zeta/uefi.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/uefi.payload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/uefi.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2023-08-01

# Andromeda888 UEFI as Boot (SecureBoot Disabled)
cat ./BootShim/BootShim.Zeta.bin ./ImageResources/Zeta/SM8350_EFI_NOSB.fd > ./ImageResources/Zeta/uefi_nosb.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/uefi_nosb.payload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/uefi_nosb.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2024-02-01