#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("Usage: %s <input>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *inputFilePath = argv[1];

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