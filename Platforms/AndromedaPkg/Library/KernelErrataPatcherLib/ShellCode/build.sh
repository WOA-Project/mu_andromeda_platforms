#!/bin/bash

cd shellcodecleaner
make
cd ..

ACTLR_EL1=$(./shellcodecleaner/main ./ACTLR_EL1/Reference/ShellCode.c)
AMCNTENSET0_EL0=$(./shellcodecleaner/main ./AMCNTENSET0_EL0/Reference/ShellCode.c)
IRM=$(./shellcodecleaner/main ./IRM/Reference/ShellCode.c)
IRM_X8=$(./shellcodecleaner/main ./IRM_X8/Reference/ShellCode.c)
PSCI_MEMPROTECT=$(./shellcodecleaner/main ./PSCI_MEMPROTECT/Reference/ShellCode.c)

cp ./ShellCodeTemplate.h ../ShellCode.h

sed -i "s/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/${ACTLR_EL1}/g" ../ShellCode.h
sed -i "s/BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB/${AMCNTENSET0_EL0}/g" ../ShellCode.h
sed -i "s/CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC/${IRM}/g" ../ShellCode.h
sed -i "s/DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD/${IRM_X8}/g" ../ShellCode.h
sed -i "s/EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE/${PSCI_MEMPROTECT}/g" ../ShellCode.h
