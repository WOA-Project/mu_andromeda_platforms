#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <input> <output>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* inputFilePath = argv[1];
    char* outputFilePath = argv[2];

    printf("Input: %s\n", inputFilePath);
    printf("Output: %s\n", outputFilePath);

    FILE *inputFile;
    FILE *outputFile;

    inputFile = fopen(inputFilePath, "r");
    if (inputFile == NULL) {
        perror("Error opening file for reading");
        return EXIT_FAILURE;
    }

    outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        perror("Error opening output file for writing");
        return EXIT_FAILURE;
    }

    char buffer[256];

    int obtainedOslArm64TransferToKernel = 0;
    int obtainedMovX29SPAfterOslArm64TransferToKernel = 0;

    int trueBeginning = 0;
    int trueEnd = 0;

    int lineIndex = 0;
    while (fgets(buffer, sizeof(buffer), inputFile) != NULL) {
        //printf("%s", buffer); // Print each line

        if (strcmp(buffer, "OslArm64TransferToKernel:\n") == 0) {
            obtainedOslArm64TransferToKernel = 1;
        }

        if (obtainedOslArm64TransferToKernel) {
            if (strcmp(buffer, "	mov	x29, sp\n") == 0) {
                obtainedMovX29SPAfterOslArm64TransferToKernel = 1;
                obtainedOslArm64TransferToKernel = 0; // Reset
                trueBeginning = lineIndex + 1;
            }
        }

        if (strcmp(buffer, "	bl	DoSomething\n") == 0) {
            trueEnd = lineIndex - 2;
        }

        lineIndex++;
    }

    // Write new file
    rewind(inputFile);

    // Print header into new file
    char* header = "/** @file\n\n  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1\n  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0\n  Patches NTOSKRNL to not cause a bugcheck when attempting to use\n  PSCI_MEMPROTECT Due to an issue in QHEE\n\n  Shell Code to patch kernel mode components before NTOSKRNL\n\n  Copyright (c) 2022-2023 DuoWoA authors\n\n  SPDX-License-Identifier: MIT\n\n**/\n\n//VOID\n//OslArm64TransferToKernel (\n//  INT VOID *OsLoaderBlock, INT *KernelAddress\n//  );\n_Start:\n";
    fprintf(outputFile, "%s", header);

    lineIndex = 0;
    int isWriting = 0;
    while (fgets(buffer, sizeof(buffer), inputFile) != NULL) {
        if (lineIndex == trueBeginning) {
            isWriting = 1;
        }

        if (isWriting) {
            if (strcmp(buffer, "	beq	.L4\n") == 0) {
                fprintf(outputFile, "	beq	_Payload\n");
            } else {
                fprintf(outputFile, "%s", buffer);
            }
        }

        if (lineIndex == trueEnd) {
            isWriting = 0;
        }

        lineIndex++;
    }

    char* footer = "\n_Dead:\n	/* We should never get here */\n	b		_Dead\n\n.text\n.align 4\n\n_Payload:\n	/* Your code will get ran right after this binary */\n";
    fprintf(outputFile, "%s", footer);

    fclose(inputFile);
    fclose(outputFile);

    return EXIT_SUCCESS;
}