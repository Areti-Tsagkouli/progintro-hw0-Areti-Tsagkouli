#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void writeBMP(FILE *out, unsigned char header[54], unsigned char *pixels, size_t size_padded) {
    fwrite(header, sizeof(unsigned char), 54, out);
    fwrite(pixels, sizeof(unsigned char), size_padded - 54, out);
}

void readBMP(FILE *in, int *width, int *height, unsigned char header[54], unsigned char **pixels, int *row_padded, size_t *size_padded) {
    size_t header_size = fread(header, sizeof(unsigned char), 54, in);
    if (header_size != 54) {
        fprintf(stderr, "Error: header could not be read\n");
        exit(EXIT_FAILURE);
    }

    *width = *(int *) &header[18];
    *height = *(int *) &header[22];
    *row_padded = (*width * 3 + 3) & (~3);
    *size_padded = (size_t)(*row_padded) * (*height);

    *pixels = (unsigned char *)malloc(*size_padded);

    size_t pixels_size = fread(*pixels, sizeof(unsigned char), *size_padded - 54, in);
    if (pixels_size != *size_padded - 54) {
        fprintf(stderr, "Error: pixels could not be read\n");
        exit(EXIT_FAILURE);
    }
}

void rotate90DegreeRectangular(int width, int height, unsigned char *inputPixels, int row_padded, unsigned char header[54]) {
    int newWidth = height;
    int newHeight = width;
    int newRowPadded = (newWidth * 3 + 3) & (~3);
    size_t newSizePadded = (size_t)newRowPadded * newHeight;

    unsigned char *outputPixels = (unsigned char *)malloc(54 + newSizePadded);

    if (outputPixels == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for rotated image\n");
        exit(EXIT_FAILURE);
    }

    // Copy the header to the outputPixels array
    memcpy(outputPixels, header, 54);

    // Update header information
    *(int*)&outputPixels[18] = newWidth;
    *(int*)&outputPixels[22] = newHeight;

    // Copy pixel data, skipping the input header
    for (int x = 0; x < newHeight; x++) {
        for (int y = 0; y < newWidth; y++) {
            outputPixels[54 + (x * newRowPadded) + (y * 3) + 0] = inputPixels[(y * row_padded) + (newHeight - x - 1) * 3 + 0];
            outputPixels[54 + (x * newRowPadded) + (y * 3) + 1] = inputPixels[(y * row_padded) + (newHeight - x - 1) * 3 + 1];
            outputPixels[54 + (x * newRowPadded) + (y * 3) + 2] = inputPixels[(y * row_padded) + (newHeight - x - 1) * 3 + 2];
        }
    }

    // Write rotated BMP to standard output
    writeBMP(stdout, outputPixels, outputPixels + 54, 54 + newSizePadded);

    free(outputPixels);
}

 void rotate90DegreeSquare(int size, unsigned char *inputPixels, unsigned char header[54]) {
    int newRowPadded = ((size * 3) + 3) & (~3);
    size_t newSizePadded = (size_t)newRowPadded * size;

    unsigned char *newPixels = (unsigned char *)malloc(54 + newSizePadded);

    if (newPixels == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for rotated image\n");
        exit(EXIT_FAILURE);
    }

    // Update header information
    memcpy(newPixels, header, 54);
    *(int*)&newPixels[18] = size;
    *(int*)&newPixels[22] = size;

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            // Rotate 90 degrees counterclockwise
            newPixels[54 + (y * newRowPadded) + (size - x - 1) * 3 + 0] = inputPixels[54 + (x * newRowPadded) + y * 3 + 0];
            newPixels[54 + (y * newRowPadded) + (size - x - 1) * 3 + 1] = inputPixels[54 + (x * newRowPadded) + y * 3 + 1];
            newPixels[54 + (y * newRowPadded) + (size - x - 1) * 3 + 2] = inputPixels[54 + (x * newRowPadded) + y * 3 + 2];
        }
    }

    // Write rotated BMP to standard output
    writeBMP(stdout, newPixels, newPixels + 54, 54 + newSizePadded);

    free(newPixels);
}



int main() {
    int width, height, row_padded;
    size_t size_padded;
    unsigned char header[54];
    unsigned char *pixels;

    // Read BMP from standard input
    readBMP(stdin, &width, &height, header, &pixels, &row_padded, &size_padded);

    // Determine if the image is square or rectangular
    if (width == height) {
        // Square image
        rotate90DegreeSquare(width, pixels, header);
    } else {
        // Rectangular image
        rotate90DegreeRectangular(width, height, pixels, row_padded, header);
    }

    free(pixels);

    return EXIT_SUCCESS;
}
