#include <stdlib.h>
#include <stdio.h>
#include "gbarom.h"
#include "../gba_system.h"

void load_gbarom(const char* filename) {
    FILE *fp = fopen(filename, "rb");

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    byte *buf = malloc(size);
    fread(buf, size, 1, fp);

    mem->rom = buf;
    mem->rom_size = size;
}
