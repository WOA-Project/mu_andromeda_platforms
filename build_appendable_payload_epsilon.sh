#!/bin/bash

OUTPUT_DIR=./ImageResources/Epsilon/UEFIAppendPayloads
mkdir -p "$OUTPUT_DIR"

CONFIG_FILE=${UEFI_CONFIG_ON:-./BootConfig/UefiPlatEpsilon.cfg}
CONFIG_SIZE=$(wc -c < "$CONFIG_FILE")
make -B -C ./BootConfig UEFI_FD_BASE=0x9FC01000 UEFI_FD_SIZE=0x002EF000 BOOT_CONFIG_SHIM_SIZE=0x1000 BOOT_CONFIG_SIZE="$CONFIG_SIZE" BootConfigShim.bin
bash ./BootConfig/build_appendable_payload.sh \
  ./ImageResources/Epsilon/SM8150_EFI.fd \
  ./BootConfig/BootConfigShim.bin \
  "$CONFIG_FILE" \
  "$OUTPUT_DIR/UefiPlatEpsilon.bin" \
  0x002EF000 \
  0x00300000

CONFIG_FILE=${UEFI_CONFIG_OFF:-./BootConfig/UefiPlatEpsilonNoSb.cfg}
CONFIG_SIZE=$(wc -c < "$CONFIG_FILE")
make -B -C ./BootConfig UEFI_FD_BASE=0x9FC01000 UEFI_FD_SIZE=0x002EF000 BOOT_CONFIG_SHIM_SIZE=0x1000 BOOT_CONFIG_SIZE="$CONFIG_SIZE" BootConfigShim.bin
bash ./BootConfig/build_appendable_payload.sh \
  ./ImageResources/Epsilon/SM8150_EFI.fd \
  ./BootConfig/BootConfigShim.bin \
  "$CONFIG_FILE" \
  "$OUTPUT_DIR/UefiPlatEpsilonNoSb.bin" \
  0x002EF000 \
  0x00300000
