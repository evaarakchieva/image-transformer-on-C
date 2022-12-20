#include "image_processor.h"
#include "util.h"

static enum read_status header_reader (FILE *in, struct bmp_header *header) {
    if (fread(header, sizeof(struct bmp_header), 1, in) == 1)
        return READ_OK;
    else {
        return READ_INVALID_HEADER;
    }
}

struct bmp_header bmp_header_generator (const struct image* img) {
    uint32_t padding;
    padding = padding_calculator(img->width);
    uint64_t height = img -> height;
    uint64_t width = img -> width;
    struct bmp_header header = {
            .bfType = 0x4D42,
            .bfReserved = 0,
            .bOffBits = sizeof(struct bmp_header),
            .biSize = 40,
            .biHeight = height,
            .biWidth = width,
            .biPlanes = 1,
            .biBitCount = 24,
            .biCompression = 0,
            .biSizeImage = sizeof(struct pixel) * width * height + padding * height,
            .bfileSize = (sizeof(struct bmp_header) + height * width * sizeof(struct pixel)
                          + height * padding)
            ,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed =0,
            .biClrImportant = 0 };
    return header;
}

enum read_status from_bmp( FILE* in, struct image* img ) {
    printf("PROCESS OF FILE READING HAS BEGUN\n");
    struct bmp_header *header = malloc(sizeof(struct bmp_header));
    enum read_status header_checkup = header_reader(in, header);
    if (header_checkup != READ_OK) {
        return header_checkup;
    }

    img->width = header->biWidth;
    img->height = header->biHeight;
    img->data = malloc(sizeof(struct pixel) * img->height * img->width);

    uint32_t padding;
    padding = padding_calculator(img->width);

    struct pixel *pixels = img->data;
    size_t width = img->width;
    size_t height = img->height;
    for (size_t i = 0; i < height; i++) {
        if (fread(pixels + width * i, sizeof(struct pixel),width,in) != width) {
            free(pixels);
            return READ_INVALID_BITS;
        }
        if (fseek(in, padding, SEEK_CUR) != 0) {
            free(pixels);
            return READ_INVALID_SIGNATURE;
        }
    }
    free(header);
    return READ_OK;
}

enum write_status to_bmp( FILE* out, struct image const* img ){
    struct bmp_header header = bmp_header_generator(img);
    if (fwrite(&header, sizeof(struct bmp_header), 1, out) != 1)
        return WRITE_ERROR;

    uint32_t padding;
    padding = padding_calculator(img->width);

    struct pixel *new_pixel = img->data;
    for (size_t i = 0; i < img->height; i++) {
        fwrite(&header, sizeof(struct bmp_header), 1, out);
        if (fwrite(new_pixel, sizeof(struct pixel), img->width, out) != img->width)
            return WRITE_ERROR;
        if (fwrite(new_pixel, 1, padding, out) != padding)
            return WRITE_ERROR;
        new_pixel = new_pixel + (size_t) img->width;
    }
    return WRITE_OK;
}