#include <limits.h>
#include <string.h>

#include "mem/gbabus.h"
#include "mem/gbarom.h"
#include "mem/gbabios.h"
#include "gba_system.h"

int cycles = 0;

arm7tdmi_t* cpu = NULL;
gba_ppu_t* ppu = NULL;
gbabus_t* bus = NULL;
gbamem_t* mem = NULL;
gba_apu_t* apu = NULL;
bool should_quit = false;

#define VISIBLE_CYCLES 960
#define HBLANK_CYCLES 272
#define VISIBLE_LINES 160
#define VBLANK_LINES 68

#define BACKUP_PERSIST_DEBOUNCE_FRAMES 10

const char* get_backup_path(const char* romfile) {
    char buf[PATH_MAX];
    char* rom_path = realpath(romfile, buf);
    int backup_path_buf_size = strlen(rom_path) + 8; // .backup + null terminator

    if (backup_path_buf_size >= PATH_MAX) {
        logfatal("Backup path too long! How deeply are you nesting your directories?")
    }

    char* backuppath = malloc(backup_path_buf_size);
    snprintf(backuppath, backup_path_buf_size, "%s.backup", buf);
    return backuppath;
}

const char* get_savestate_path(const char* romfile, int number) {
    char buf[PATH_MAX];
    char* rom_path = realpath(romfile, buf);
    int savestate_path_buf_size = strlen(rom_path) + 9; // .xx.save + null terminator

    if (savestate_path_buf_size >= PATH_MAX) {
        logfatal("Savestate path too long! How deeply are you nesting your directories?")
    }

    char* savestate_path = malloc(savestate_path_buf_size);
    snprintf(savestate_path, savestate_path_buf_size, "%s.%02d.save", buf, number);
    return savestate_path;
}

void init_gbasystem(const char* romfile, const char* bios_file) {
    mem = init_mem();

    load_gbarom(romfile);
    mem->backup_path = get_backup_path(romfile);
    mem->savestate_path = malloc(NUM_SAVESTATES * sizeof(char*));
    for (int i = 0; i < NUM_SAVESTATES; i++) {
        mem->savestate_path[i] = get_savestate_path(romfile, i + 1);
    }
    if (bios_file) {
        load_alternate_bios(bios_file);
    }

    // Initialize the CPU, hook it up to the GBA bus
    cpu = init_arm7tdmi(gba_read_byte,
                        gba_read_half,
                        gba_read_word,
                        gba_write_byte,
                        gba_write_half,
                        gba_write_word);

    ppu = init_ppu();
    bus = init_gbabus();
    apu = init_apu();
}


// Unused, since everything relies on shifts and masks for faster mods and divides, but kept here for posterity.
// int timer_freq[4] = {1, 64, 256, 1024};
// Optimizing division
int timer_shift[4] = {0, 6, 8, 10};
// Optimizing modulus
int timer_mask[4] = {0, 0x3F, 0xFF, 0x3FF};

void timer_tick(int cyc) {
    bool previous_overflow = false;
    for (int n = 0; n < 4; n++) {
        bool overflow = false;

        if (bus->TMSTART[n]) {
            if (bus->TMCNT_H[n].cascade && !previous_overflow) {
                continue;
            }

            if (bus->TMCNT_H[n].cascade && !previous_overflow) {
                previous_overflow = false;
                continue; // Don't inc
            }

            if (bus->TMCNT_H[n].cascade) {
                bus->TMINT[n].ticks += 1;
            } else {
                bus->TMINT[n].ticks += cyc;
            }

            word clocks = bus->TMINT[n].ticks >> timer_shift[bus->TMCNT_H[n].frequency];
            word remain = bus->TMINT[n].ticks & timer_mask[bus->TMCNT_H[n].frequency];
            word new_value = bus->TMINT[n].value + clocks;

            while (new_value > 0xFFFF) {
                new_value = bus->TMCNT_L[n].timer_reload + (new_value - 0xFFFF);
                overflow = true;
                if (bus->TMCNT_H[n].timer_irq_enable) {
                    switch (n) {
                        case 0:
                            request_interrupt(IRQ_TIMER0);
                            break;
                        case 1:
                            request_interrupt(IRQ_TIMER1);
                            break;
                        case 2:
                            request_interrupt(IRQ_TIMER2);
                            break;
                        case 3:
                            request_interrupt(IRQ_TIMER3);
                            break;
                    }
                }

                if (n == 0 || n == 1) {
                    sound_timer_overflow(apu, n);
                }
            }
            bus->TMINT[n].value = new_value;
            bus->TMINT[n].ticks = remain;
        }

        previous_overflow = overflow;
    }
}

bool cpu_stepped = false;

INLINE int inline_gba_cpu_step() {
    cpu_stepped = false;
    cpu->irq = (bus->interrupt_enable.raw & bus->IF.raw) != 0;
    if (cpu->halt && !cpu->irq) {
        return 1;
    } else {
        cpu_stepped = true;
        return arm7tdmi_step(cpu);
    }
}

INLINE int run_system(int for_cycles) {
    while (for_cycles > 0) {
        int ran = inline_gba_cpu_step();
        timer_tick(ran);
        for (int i = 0; i < ran; i++) {
            apu_tick(apu);
        }
        for_cycles -= ran;
    }
    return for_cycles;
}

INLINE void inline_gba_system_step() {
    int this_step_cycles = inline_gba_cpu_step();
    timer_tick(this_step_cycles);
    cycles += this_step_cycles;
    while (cycles > 0) {
        apu_tick(apu);
        cycles--;
    }
}

// Non-inlined version of the above
void gba_system_step() {
    inline_gba_system_step();
}

void persist_backup() {
    if (mem->backup_dirty) {
        mem->backup_persist_countdown = BACKUP_PERSIST_DEBOUNCE_FRAMES;
        mem->backup_dirty = 0;
    } else if (--mem->backup_persist_countdown == 0) {
        if (bus->backup_type != UNKNOWN && mem->backup != NULL) {
            FILE *fp = fopen(mem->backup_path, "wb");
            if (fp != NULL) {
                fwrite(mem->backup, mem->backup_size, 1, fp);
                fclose(fp);
            }
            mem->backup_dirty = false;
        }
    }
}

void cleanup() {
    free(mem->backup);
    free((void*)mem->backup_path);
    for (int i = 0; i < 10; i++) {
        free((void*)mem->savestate_path[i]);
    }
    free((void*)mem->savestate_path);
    free(mem->rom);
    free(mem);
    mem = NULL;

    free(ppu);
    ppu = NULL;
    free(bus);
    bus = NULL;
    free(apu);
    apu = NULL;
    free(cpu);
    cpu = NULL;
}

void gba_system_loop() {
    int extra = 0;
    while (!should_quit) {
        for (int line = 0; line < VISIBLE_LINES; line++) {
            extra = run_system(VISIBLE_CYCLES + extra);
            ppu_hblank(ppu);
            extra = run_system(HBLANK_CYCLES + extra);
            ppu_end_hblank(ppu);
        }
        ppu_vblank(ppu);
        for (int line = 0; line < VBLANK_LINES; line++) {
            extra = run_system(VISIBLE_CYCLES + extra);
            ppu_hblank(ppu);
            extra = run_system(HBLANK_CYCLES + extra);
            ppu_end_hblank(ppu);
        }
        ppu_end_vblank(ppu);
        persist_backup();
    }

    cleanup();
}

typedef struct savestate_header {
    char magic[3]; // Must be "DGB"
    size_t cpu_size;
    size_t ppu_size;
    size_t bus_size;
    size_t mem_size;
    size_t backup_size;
    size_t apu_size;
} savestate_header_t;

void save_state(const char* path) {
    savestate_header_t header;
    header.magic[0] = 'D';
    header.magic[1] = 'G';
    header.magic[2] = 'B';
    header.cpu_size = sizeof(arm7tdmi_t);
    header.ppu_size = sizeof(gba_ppu_t);
    header.bus_size = sizeof(gbabus_t);
    header.mem_size = sizeof(gbamem_t);
    header.backup_size = mem->backup_size;
    header.apu_size = sizeof(gba_apu_t);

    FILE* fp = fopen(path, "wb");

    fwrite(&header, sizeof(savestate_header_t), 1, fp);
    fwrite(cpu, header.cpu_size, 1, fp);
    fwrite(ppu, header.ppu_size, 1, fp);
    fwrite(bus, header.bus_size, 1, fp);
    fwrite(mem, header.mem_size, 1, fp);
    fwrite(mem->backup, header.backup_size, 1, fp);
    fwrite(apu, header.apu_size, 1, fp);
    fclose(fp);
}

// TODO: make sure we don't read more bytes than we have space for
void load_state(const char* path) {
    FILE* fp = fopen(path, "rb");
    savestate_header_t header;
    fread(&header, sizeof(savestate_header_t), 1, fp);

    // Restore CPU. Need to restore function pointers.
    fread(cpu, header.cpu_size, 1, fp);
    cpu->read_byte = &gba_read_byte;
    cpu->read_half = &gba_read_half;
    cpu->read_word = &gba_read_word;

    cpu->write_byte = &gba_write_byte;
    cpu->write_half = &gba_write_half;
    cpu->write_word = &gba_write_word;

    // Restore PPU. No pointers need to be restored.
    fread(ppu, header.ppu_size, 1, fp);

    // Restore bus. No pointers need to be restored.
    fread(bus, header.bus_size, 1, fp);

    // Restore mem. Need to restore ROM and backup space + paths.
    byte* rom = mem->rom;
    const char* backup_path = mem->backup_path;
    const char** savestate_path = mem->savestate_path;
    byte* backup = mem->backup;
    fread(mem, header.mem_size, 1, fp);
    mem->rom = rom;
    mem->backup_path = backup_path;
    mem->savestate_path = savestate_path;
    mem->backup = backup;
    fread(mem->backup, header.backup_size, 1, fp);

    // Restore APU. No pointers need to be restored.
    fread(apu, header.apu_size, 1, fp);
}