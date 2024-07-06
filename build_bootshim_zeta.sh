#!/bin/bash

# Andromeda888 BootShim
cd BootShim
make UEFI_BASE=0x9FC41000 UEFI_SIZE=0x002BF000
mv ./BootShim.bin ./BootShim.Zeta.bin
mv ./BootShim.elf ./BootShim.Zeta.elf
cd ..