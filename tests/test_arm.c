#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/common/log.h"
#include "../src/arm7tdmi/arm7tdmi.h"
#include "../src/gbabus.h"
#include "../src/gbarom.h"
#include "../src/gbabios.h"

bool in_bios(arm7tdmi_t* state) {
    return state->pc < GBA_BIOS_SIZE;
}

#define TEST_FAILED_ADDRESS 0x1B94
#define NUM_LOG_LINES 1107
#define LOG_FILE "arm.log"

typedef struct cpu_log {
    word address;
    word instruction;
    status_register_t cpsr;
    word r[16];
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

        fseek(fp, 1, SEEK_CUR); // Skip newline character

        buffer[l] = log;
    }
}

#define ASSERT_EQUAL(message, expected, actual) \
    if (expected != actual) { \
        logfatal("ASSERTION FAILED: " message " expected: 0x%08X != actual: 0x%08X\n", expected, actual); }

#define cpsrflag(f, c) (f == 1?c:"-")
#define printcpsr(cpsr) printf("[%s%s%s%s%s%s%s]", cpsrflag(cpsr.N, "N"), cpsrflag(cpsr.Z, "Z"), \
         cpsrflag(cpsr.C, "C"), cpsrflag(cpsr.V, "V"), cpsrflag(cpsr.disable_irq, "I"), \
         cpsrflag(cpsr.disable_fiq, "F"), cpsrflag(cpsr.thumb, "T"))

int main(int argc, char** argv) {
    log_set_verbosity(4);
    gbamem_t* mem = init_mem();
    init_gbabus(mem);

    load_gbarom("arm.gba", mem);

    // Initialize the CPU, hook it up to the GBA bus
    arm7tdmi_t* cpu = init_arm7tdmi(gba_read_byte, gba_read_half, gba_read_word,
                                    gba_write_byte, gba_write_half, gba_write_word);
    skip_bios(cpu);

    cpu_log_t lines[NUM_LOG_LINES];

    load_log(LOG_FILE, NUM_LOG_LINES, lines);

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    int step = 0;

    while(true) {
        // Register values in the log are BEFORE EXECUTING the instruction on that line
        logdebug("Checking registers against step %d (line %d in log)", step, step + 1)
        ASSERT_EQUAL("Address", lines[step].address, cpu->pc - (cpu->cpsr.thumb ? 2 : 4))

        if (lines[step].cpsr.raw != cpu->cpsr.raw) {
            printf("Expected cpsr: ");
            printcpsr(lines[step].cpsr);
            printf(" Actual cpsr: ");
            printcpsr(cpu->cpsr);
            printf("\n");
        }

        ASSERT_EQUAL("r0",   lines[step].r[0],     get_register(cpu, 0))
        ASSERT_EQUAL("r1",   lines[step].r[1],     get_register(cpu, 1))
        ASSERT_EQUAL("r2",   lines[step].r[2],     get_register(cpu, 2))
        ASSERT_EQUAL("r3",   lines[step].r[3],     get_register(cpu, 3))
        ASSERT_EQUAL("r4",   lines[step].r[4],     get_register(cpu, 4))
        ASSERT_EQUAL("r5",   lines[step].r[5],     get_register(cpu, 5))
        ASSERT_EQUAL("r6",   lines[step].r[6],     get_register(cpu, 6))
        ASSERT_EQUAL("r7",   lines[step].r[7],     get_register(cpu, 7))
        ASSERT_EQUAL("r8",   lines[step].r[8],     get_register(cpu, 8))
        ASSERT_EQUAL("r9",   lines[step].r[9],     get_register(cpu, 9))
        ASSERT_EQUAL("r10",  lines[step].r[10],    get_register(cpu, 10))
        ASSERT_EQUAL("r11",  lines[step].r[11],    get_register(cpu, 11))
        ASSERT_EQUAL("r12",  lines[step].r[12],    get_register(cpu, 12))
        ASSERT_EQUAL("r13",  lines[step].r[13],    get_register(cpu, 13))
        ASSERT_EQUAL("r14",  lines[step].r[14],    get_register(cpu, 14))
        ASSERT_EQUAL("r15",  lines[step].r[15],    get_register(cpu, 15))
        ASSERT_EQUAL("CPSR", lines[step].cpsr.raw, cpu->cpsr.raw)

        arm7tdmi_step(cpu);
        ASSERT_EQUAL("instruction", lines[step].instruction, cpu->instr)
        step++;

        if (cpu->pc == TEST_FAILED_ADDRESS + 8) {
            word failed_test = cpu->r[12];
            logfatal("test_arm: FAILED TEST: %d", failed_test)
        }
        // TODO check for when all tests pass
    }
    exit(0);
}
