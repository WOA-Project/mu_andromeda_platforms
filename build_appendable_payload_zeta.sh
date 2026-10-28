#!/bin/bash

OUTPUT_DIR=./ImageResources/Zeta/UEFIAppendPayloads
mkdir -p "$OUTPUT_DIR"

CONFIG_FILE=${UEFI_CONFIG_ON:-./BootConfig/UefiPlatZeta.cfg}
CONFIG_SIZE=$(wc -c < "$CONFIG_FILE")
make -B -C ./BootConfig UEFI_FD_BASE=0x9FC42000 UEFI_FD_SIZE=0x002AE000 BOOT_CONFIG_SHIM_SIZE=0x1000 BOOT_CONFIG_SIZE="$CONFIG_SIZE" BootConfigShim.bin
bash ./BootConfig/build_appendable_payload.sh \
  ./ImageResources/Zeta/SM8350_EFI.fd \
  ./BootConfig/BootConfigShim.bin \
  "$CONFIG_FILE" \
  "$OUTPUT_DIR/UefiPlatZeta.bin" \
  0x002AE000 \
  0x002BF000

CONFIG_FILE=${UEFI_CONFIG_OFF:-./BootConfig/UefiPlatZetaNoSb.cfg}
CONFIG_SIZE=$(wc -c < "$CONFIG_FILE")
make -B -C ./BootConfig UEFI_FD_BASE=0x9FC42000 UEFI_FD_SIZE=0x002AE000 BOOT_CONFIG_SHIM_SIZE=0x1000 BOOT_CONFIG_SIZE="$CONFIG_SIZE" BootConfigShim.bin
bash ./BootConfig/build_appendable_payload.sh \
  ./ImageResources/Zeta/SM8350_EFI.fd \
  ./BootConfig/BootConfigShim.bin \
  "$CONFIG_FILE" \
  "$OUTPUT_DIR/UefiPlatZetaNoSb.bin" \
  0x002AE000 \
  0x002BF000
