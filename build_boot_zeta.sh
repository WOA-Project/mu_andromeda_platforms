#!/bin/bash

# Andromeda888 UEFI as Boot (Dual Boot)
cat ./ImageResources/Zeta/patchedkernel ./ImageResources/Zeta/SM8350_EFI.fd > ./ImageResources/Zeta/boot.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/boot.payload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/boot.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2024-04-01

# Andromeda888 UEFI as Boot (Dual Boot) (SecureBoot Disabled)
cat ./ImageResources/Zeta/patchedkernel ./ImageResources/Zeta/SM8350_EFI_NOSB.fd > ./ImageResources/Zeta/boot_nosb.payload.bin
python3 ./ImageResources/mkbootimg.py \
  --kernel ./ImageResources/Zeta/boot_nosb.payload.bin \
  --ramdisk ./ImageResources/Zeta/ramdisk \
  -o ./ImageResources/Zeta/boot_nosb.img \
  --pagesize 4096 \
  --header_version 3 \
  --cmdline "" \
  --base 0x0 \
  --os_version 11.0.0 \
  --os_patch_level 2024-04-01