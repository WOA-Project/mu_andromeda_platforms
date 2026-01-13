#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CROSS_COMPILER "aarch64-linux-gnu"
#define ENTRY_POINT "DoSomething"
#define EXIT_POINT "OslArm64TransferToKernel"

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

    int obtainedExitPoint = 0;
    int obtainedMovX29SPAfterExitPoint = 0;

    int trueBeginning = 0;
    int trueEnd = 0;

    int lineIndex = 0;
    while (fgets(buffer, sizeof(buffer), inputFile) != NULL) {
        //printf("%s", buffer); // Print each line

        if (strcmp(buffer, EXIT_POINT ":\n") == 0) {
            obtainedExitPoint = 1;
        }

        if (obtainedExitPoint) {
            if (strcmp(buffer, "	mov	x29, sp\n") == 0) {
                obtainedMovX29SPAfterExitPoint = 1;
                obtainedExitPoint = 0; // Reset
                trueBeginning = lineIndex + 1;
            }
        }

        if (strcmp(buffer, "	bl	" ENTRY_POINT "\n") == 0) {
            trueEnd = lineIndex - 2;
        }

        lineIndex++;
    }

    // Write new file
    rewind(inputFile);

    // Print header into new file
    char* header = "_Start:\n";
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

    char* footer = "\n_Dead:\n	b		_Dead\n.text\n.align 4\n_Payload:";
    fprintf(outputFile, "%s", footer);

    fclose(inputFile);
    fclose(outputFile);

    return EXIT_SUCCESS;
}

void compile_c_to_assembly_raw(char* inputFile, char* outputFile) {
    char compileCommand1[256] = {0};

    sprintf(compileCommand1, CROSS_COMPILER "-gcc -S -o %s -O1 -fno-stack-protector %s", outputFile, inputFile);

    system(compileCommand1);
}

void compile_assembly_to_opcode(char* inputFile, char* outputFile) {
    char compileCommand1[256] = {0};
    char compileCommand2[256] = {0};

    sprintf(compileCommand1, CROSS_COMPILER "-gcc -c %s -o ShellCode.elf", inputFile);
    sprintf(compileCommand2, CROSS_COMPILER "-objcopy -O binary ShellCode.elf %s", outputFile);

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