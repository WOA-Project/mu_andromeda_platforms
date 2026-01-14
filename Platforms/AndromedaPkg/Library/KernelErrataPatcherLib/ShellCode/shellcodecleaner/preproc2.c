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

    char character;

    char* matchStart = "/*@@@@@@ARMTOLE{";
    char* matchEnd = "}@@@@@@*/0";

    int isInSequence = 0;
    int sequenceStart = 0;
    int sequenceEnd = 0;

    char buffer[256] = {0};

    int charIndex = 0;
    while (!feof(inputFile)) {
        fseek(inputFile, charIndex, SEEK_SET);
        character = getc(inputFile);

        if (feof(inputFile)) {
            break;
        }

        if (isInSequence == 0) {
            fseek(inputFile, charIndex, SEEK_SET);

            for (int j = 0; j < strlen(matchStart); j++) {
                if (feof(inputFile)) {
                    break;
                }

                character = getc(inputFile);
                if (character != matchStart[j]) {
                    break;
                }

                if (j + 1 == strlen(matchStart)) {
                    sequenceStart = charIndex;
                    isInSequence = 1;
                }
            }

            if (!feof(inputFile)) {
                fseek(inputFile, charIndex, SEEK_SET);
                character = getc(inputFile);
            }
        } else {
            fseek(inputFile, charIndex, SEEK_SET);

            for (int j = 0; j < strlen(matchEnd); j++) {
                if (feof(inputFile)) {
                    break;
                }

                character = getc(inputFile);
                if (character != matchEnd[j]) {
                    break;
                }

                if (j + 1 == strlen(matchEnd)) {
                    sequenceEnd = charIndex + strlen(matchEnd) - 1;
                    isInSequence = 0;

                    //printf("Sequence found at: %i-%i\n", sequenceStart, sequenceEnd);
                    fseek(inputFile, sequenceStart + strlen(matchStart), SEEK_SET);

                    fread(buffer, sizeof(char), sequenceEnd - sequenceStart + 1, inputFile);
                    buffer[sequenceEnd - sequenceStart - strlen(matchStart) - strlen(matchEnd) + 1] = '\0';

                    // Buffer contains the command line arguments to pass to armtolle now
                    //printf("%s\n", buffer);

                    char command[1035] = {0};
                    char output[1035] = {0};

                    sprintf(command, "./armtole \"%s\"", buffer);

                    FILE *fp = popen(command, "r");
                    if (fp == NULL) {
                        printf("Failed to run armtolle\n");
                        return EXIT_FAILURE;
                    }

                    // TODO: Handle multiline
                    while (fgets(output, sizeof(output), fp) != NULL) {
                        // Remove trailing new line
                        output[strlen(output) - 1] = '\0';
                        //printf("%s", output);
                        fputs(output, outputFile);
                    }

                    pclose(fp);

                    charIndex = sequenceEnd + 1;

                    fseek(inputFile, charIndex, SEEK_SET);
                    character = getc(inputFile);
                }
            }

            if (!feof(inputFile)) {
                fseek(inputFile, charIndex, SEEK_SET);
                character = getc(inputFile);
            }
        }

        if (!isInSequence) {
            //printf("%c", character);
            putc(character, outputFile);
        }

        //printf("%c", character);

        charIndex++;
    }

    return EXIT_SUCCESS;
}