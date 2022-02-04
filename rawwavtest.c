#include "./include/adpcm_decoder.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main() {
    decoder_reset();
    FILE* input = fopen("input.ima", "rb");
    FILE* output = fopen("output.pcm", "wb");

    if (input == 0 || output == 0) {
        printf("failed to open file");
        return -1;
    }

    fseek(input, 0, SEEK_END); // Jump to the end of the file
    long filelen = ftell(input); // Get the current byte offset in the file
    rewind(input);

    char* buffer = (char*)malloc(filelen * sizeof(char)); // Enough memory for the file
    fread(buffer, filelen, 1, input); // Read in the entire file
    fclose(input); // Close the file

    for (long i = 0; i < filelen; i++) {
        uint8_t byte_ = (uint8_t)buffer[i];

        uint8_t nibble_1 = byte_ >> 4;
        uint8_t nibble_2 = byte_ & 15;

        int16_t sample = decode_sample(nibble_1);
        fwrite(&sample, 2, 1, output);

        sample = decode_sample(nibble_2);
        fwrite(&sample, 2, 1, output);
    }

    fclose(output);

    return 0;
}