#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int cleanup_assembly(char* inputFilePath, char* outputFilePath) {
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
    int occurence = 0;
    while (fgets(buffer, sizeof(buffer), inputFile) != NULL) {
        if (lineIndex == trueBeginning) {
            isWriting = 1;
        }

        if (isWriting) {
            if (strcmp(buffer, "	beq	.L4\n") == 0) {
                occurence++;
                if (occurence == 1) {
                    fprintf(outputFile, "	beq	_Payload\n");
                } else {
                    fprintf(outputFile, "	b	_Payload\n");
                }
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

void compile_c_to_assembly_raw(char* inputFile, char* outputFile) {
    char compileCommand1[256] = {0};

    sprintf(compileCommand1, "aarch64-linux-gnu-gcc -S -o %s -O1 -fno-stack-protector %s", outputFile, inputFile);

    system(compileCommand1);
}

void compile_assembly_to_opcode(char* inputFile, char* outputFile) {
    char compileCommand1[256] = {0};
    char compileCommand2[256] = {0};

    sprintf(compileCommand1, "aarch64-linux-gnu-gcc -c %s -o ShellCode.elf", inputFile);
    sprintf(compileCommand2, "aarch64-linux-gnu-objcopy -O binary ShellCode.elf %s", outputFile);

    system(compileCommand1);
    system(compileCommand2);

    remove("ShellCode.elf");
}

int print_binary_as_hex_define(char* inputFilePath) {
  FILE *inputFile;

  inputFile = fopen(inputFilePath, "rb");
  if (inputFile == NULL) {
    perror("Error opening file for reading");
    return EXIT_FAILURE;
  }

  unsigned char buffer[12];
  size_t        bytesRead = 0;
  int           ranOnce   = 0;

  do {
    bytesRead = fread(buffer, sizeof(unsigned char), sizeof(buffer), inputFile);

    if (bytesRead != 0) {
      if (ranOnce) {
        printf(", \\\\\\\n");
      }
      ranOnce = 1;
    }

    printf("    ");

    for (size_t i = 0; i < bytesRead; i++) {
      if (i == bytesRead - 1) {
        printf("0x%02X", buffer[i]);
      }
      else {
        printf("0x%02X, ", buffer[i]);
      }
    }
  } while (bytesRead != 0);

  printf("\n");
  fclose(inputFile);

  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <input>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* inputFilePath = argv[1];

    compile_c_to_assembly_raw(inputFilePath, "test.S");

    cleanup_assembly("test.S", "test.cleaned.S");

    // Cleanup old temporary work file
    remove("test.S");

    compile_assembly_to_opcode("test.cleaned.S", "test.bin");

    // Cleanup old temporary work file
    remove("test.cleaned.S");

    print_binary_as_hex_define("test.bin");

    // Cleanup old temporary work file
    remove("test.bin");

    return EXIT_SUCCESS;
}