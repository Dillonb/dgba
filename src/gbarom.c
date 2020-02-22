#include <stdlib.h>
#include <stdio.h>
#include "gbarom.h"

void load_gbarom(char* filename, gbamem* mem) {
    FILE *fp = fopen(filename, "rb");

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    byte *buf = malloc(size);
    fread(buf, size, 1, fp);

    mem->rom = buf;
    mem->rom_size = size;
}
