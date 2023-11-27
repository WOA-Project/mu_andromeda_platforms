#!/bin/bash

# Andromeda855 BootShim
cd BootShim
make UEFI_BASE=0x9FC00000 UEFI_SIZE=0x00300000
mv ./BootShim.bin ./BootShim.Epsilon.bin
mv ./BootShim.elf ./BootShim.Epsilon.elf
cd ..