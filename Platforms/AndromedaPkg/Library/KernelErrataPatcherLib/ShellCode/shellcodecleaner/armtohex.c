#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CROSS_COMPILER "aarch64-linux-gnu"

int write_assembly(char* input, char* outputFilePath) {
    FILE *outputFile;

    outputFile = fopen(outputFilePath, "w");
    if (outputFile == NULL) {
        perror("Error opening output file for writing");
        return EXIT_FAILURE;
    }

    /*// Print header into new file
    char* header = "_Start:\n";
    fprintf(outputFile, "%s", header);*/

    fprintf(outputFile, "%s", input);

    /*char* footer = "\n_Dead:\n	b		_Dead\n.text\n.align 4\n_Payload:";
    fprintf(outputFile, "%s", footer);*/

    fclose(outputFile);

    return EXIT_SUCCESS;
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

    char* inputData = argv[1];

    write_assembly(inputData, "test.S");

    compile_assembly_to_opcode("test.S", "test.bin");

    // Cleanup old temporary work file
    remove("test.S");

    print_binary_as_hex_define("test.bin");

    // Cleanup old temporary work file
    remove("test.bin");

    return EXIT_SUCCESS;
}