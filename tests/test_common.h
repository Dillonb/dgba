#ifndef GBA_TEST_COMMON_H
#define GBA_TEST_COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include "../src/common/log.h"
#include "../src/gbabus.h"
#include "../src/gbarom.h"
#include "../src/gbabios.h"
#include "../src/arm7tdmi/arm7tdmi.h"

bool in_bios(arm7tdmi_t* state) {
    return state->pc < GBA_BIOS_SIZE;
}

typedef struct cpu_log {
    word address;
    word instruction;
    status_register_t cpsr;
    word r[16];
    word cycles;
} cpu_log_t;

void load_log(const char* filename, int lines, cpu_log_t* buffer) {
    FILE* fp = fopen(filename, "rb");
    char buf[10];
    for (int l = 0; l < lines; l++) {
        cpu_log_t log;
        fseek(fp, 2, SEEK_CUR); // Skip 0x
        fgets(buf, 9, fp);
        log.address = strtol(buf, NULL, 16);

        fseek(fp, 3, SEEK_CUR); // skip ,0x
        fgets(buf, 9, fp);
        log.instruction = strtol(buf, NULL, 16);

        fseek(fp, 3, SEEK_CUR); // skip ,0x
        fgets(buf, 9, fp);
        log.cpsr.raw = strtol(buf, NULL, 16);

        for (int r = 0; r < 16; r++) {
            fseek(fp, 3, SEEK_CUR); // skip ,0x
            fgets(buf, 9, fp);
            log.r[r] = strtol(buf, NULL, 16);
        }

        fseek(fp, 1, SEEK_CUR); // skip ,
        fgets(buf, 4, fp);
        log.cycles = strtol(buf, NULL, 10);

        fseek(fp, 1, SEEK_CUR); // Skip newline character

        buffer[l] = log;
    }
}

#define ASSERT_EQUAL(adjpc, message, expected, actual) \
    if (expected != actual) { \
        logfatal("ASSERTION FAILED: " message \
        " expected: 0x%08X != actual: 0x%08X" \
        " adjPC: 0x%08X" \
        "\n", expected, actual, adjpc); }

#define cpsrflag(f, c) (f == 1?c:"-")
#define printcpsr(cpsr) printf("[%s%s%s%s%s%s%s]", cpsrflag(cpsr.N, "N"), cpsrflag(cpsr.Z, "Z"), \
         cpsrflag(cpsr.C, "C"), cpsrflag(cpsr.V, "V"), cpsrflag(cpsr.disable_irq, "I"), \
         cpsrflag(cpsr.disable_fiq, "F"), cpsrflag(cpsr.thumb, "T"))

int test_loop(const char* rom_filename, int num_log_lines, const char* log_filename, word test_failed_address) {
    log_set_verbosity(4);
    gbamem_t* mem = init_mem();

    load_gbarom(rom_filename, mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi_t* cpu = init_arm7tdmi(gba_read_byte, gba_read_half, gba_read_word,
                                    gba_write_byte, gba_write_half, gba_write_word);
    gba_ppu_t* ppu = init_ppu();
    init_gbabus(mem, cpu, ppu);
    skip_bios(cpu);

    cpu_log_t lines[num_log_lines];

    load_log(log_filename, num_log_lines, lines);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    int step = 0;
    int cycles = 0;

    while(true) {
        if (step >= num_log_lines) {
            return 0; // We win!
        }
        word adjusted_pc = cpu->pc - (cpu->cpsr.thumb ? 4 : 8);
        // Register values in the log are BEFORE EXECUTING the instruction on that line
        logdebug("Checking registers against step %d (line %d in log)", step, step + 1)
        ASSERT_EQUAL(adjusted_pc, "Address", lines[step].address, cpu->pc - (cpu->cpsr.thumb ? 2 : 4))

        if (lines[step].cpsr.raw != cpu->cpsr.raw) {
            printf("Expected cpsr: ");
            printcpsr(lines[step].cpsr);
            printf(" Actual cpsr: ");
            printcpsr(cpu->cpsr);
            printf("\n");
        }

        ASSERT_EQUAL(adjusted_pc, "r0",        lines[step].r[0],     get_register(cpu, 0))
        ASSERT_EQUAL(adjusted_pc, "r1",        lines[step].r[1],     get_register(cpu, 1))
        ASSERT_EQUAL(adjusted_pc, "r2",        lines[step].r[2],     get_register(cpu, 2))
        ASSERT_EQUAL(adjusted_pc, "r3",        lines[step].r[3],     get_register(cpu, 3))
        ASSERT_EQUAL(adjusted_pc, "r4",        lines[step].r[4],     get_register(cpu, 4))
        ASSERT_EQUAL(adjusted_pc, "r5",        lines[step].r[5],     get_register(cpu, 5))
        ASSERT_EQUAL(adjusted_pc, "r6",        lines[step].r[6],     get_register(cpu, 6))
        ASSERT_EQUAL(adjusted_pc, "r7",        lines[step].r[7],     get_register(cpu, 7))
        ASSERT_EQUAL(adjusted_pc, "r8",        lines[step].r[8],     get_register(cpu, 8))
        ASSERT_EQUAL(adjusted_pc, "r9",        lines[step].r[9],     get_register(cpu, 9))
        ASSERT_EQUAL(adjusted_pc, "r10",       lines[step].r[10],    get_register(cpu, 10))
        ASSERT_EQUAL(adjusted_pc, "r11",       lines[step].r[11],    get_register(cpu, 11))
        ASSERT_EQUAL(adjusted_pc, "r12",       lines[step].r[12],    get_register(cpu, 12))
        ASSERT_EQUAL(adjusted_pc, "r13 (SP)",  lines[step].r[13],    get_register(cpu, 13))
        ASSERT_EQUAL(adjusted_pc, "r14 (LR)",  lines[step].r[14],    get_register(cpu, 14))
        ASSERT_EQUAL(adjusted_pc, "r15 (PC)",  lines[step].r[15],    get_register(cpu, 15))
        ASSERT_EQUAL(adjusted_pc, "CPSR",      lines[step].cpsr.raw, cpu->cpsr.raw)

        //ASSERT_EQUAL(adjusted_pc, "cycles", lines[step].cycles, cycles)
        if (cycles != lines[step].cycles) {
            logwarn("Cycles incorrect!")
        }

        cycles = arm7tdmi_step(cpu);
        ASSERT_EQUAL(adjusted_pc, "instruction", lines[step].instruction, cpu->instr)
        step++;

        if (cpu->pc == test_failed_address + 8) {
            word failed_test = cpu->r[12];
            logfatal("test_arm: FAILED TEST: %d", failed_test)
        }
    }
}

#endif //GBA_TEST_COMMON_H

