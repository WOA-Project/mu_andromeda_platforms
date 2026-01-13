#!/bin/bash

cd ACTLR_EL1/Reference
make
cd ../..

cd AMCNTENSET0_EL0/Reference
make
cd ../..

# Commented out for now
#
# cd IRM/Reference
# make
# cd ../..
# 
# cd IRM_X8/Reference
# make
# cd ../..

cd PSCI_MEMPROTECT/Reference
make
cd ../..

cd shellcodecleaner
make
cd ..

./shellcodecleaner/main ./ACTLR_EL1/Reference/ShellCode.S ./ACTLR_EL1/ShellCode.S
./shellcodecleaner/main ./AMCNTENSET0_EL0/Reference/ShellCode.S ./AMCNTENSET0_EL0/ShellCode.S
# ./shellcodecleaner/main ./IRM/Reference/ShellCode.S ./IRM/ShellCode.S
# ./shellcodecleaner/main ./IRM_X8/Reference/ShellCode.S ./IRM_X8/ShellCode.S
./shellcodecleaner/main ./PSCI_MEMPROTECT/Reference/ShellCode.S ./PSCI_MEMPROTECT/ShellCode.S


./shellcodecleaner/main2 ./ACTLR_EL1/Reference/ShellCode.S ./ACTLR_EL1/ShellCode.WithoutLoop.S
./shellcodecleaner/main2 ./AMCNTENSET0_EL0/Reference/ShellCode.S ./AMCNTENSET0_EL0/ShellCode.WithoutLoop.S
# ./shellcodecleaner/main2 ./IRM/Reference/ShellCode.S ./IRM/ShellCode.WithoutLoop.S
# ./shellcodecleaner/main2 ./IRM_X8/Reference/ShellCode.S ./IRM_X8/ShellCode.WithoutLoop.S
./shellcodecleaner/main2 ./PSCI_MEMPROTECT/Reference/ShellCode.S ./PSCI_MEMPROTECT/ShellCode.WithoutLoop.S



cd ACTLR_EL1
make
cd ..

cd AMCNTENSET0_EL0
make
cd ..

cd IRM
make
cd ..

cd IRM_X8
make
cd ..

cd PSCI_MEMPROTECT
make
cd ..


ACTLR_EL1=$(./shellcodecleaner/main3 ./ACTLR_EL1/ShellCode.bin)
AMCNTENSET0_EL0=$(./shellcodecleaner/main3 ./AMCNTENSET0_EL0/ShellCode.bin)
IRM=$(./shellcodecleaner/main3 ./IRM/ShellCode.bin)
IRM_X8=$(./shellcodecleaner/main3 ./IRM_X8/ShellCode.bin)
PSCI_MEMPROTECT=$(./shellcodecleaner/main3 ./PSCI_MEMPROTECT/ShellCode.bin)

cp ./ShellCodeTemplate.h ../ShellCode.h

sed -i "s/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/${ACTLR_EL1}/g" ../ShellCode.h
sed -i "s/BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB/${AMCNTENSET0_EL0}/g" ../ShellCode.h
sed -i "s/CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC/${IRM}/g" ../ShellCode.h
sed -i "s/DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD/${IRM_X8}/g" ../ShellCode.h
sed -i "s/EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE/${PSCI_MEMPROTECT}/g" ../ShellCode.h
