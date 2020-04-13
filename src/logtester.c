#include <cflags.h>
#include "common/util.h"
#include "arm7tdmi/arm7tdmi.h"
#include "common/log.h"
#include "mem/gbamem.h"
#include "mem/gbarom.h"
#include "mem/gbabus.h"
#include "gba_system.h"

typedef struct cpu_log {
    word address;
    //word instruction;
    status_register_t cpsr;
    word r[16];
    //word cycles;
} cpu_log_t;


void load_log(const char* filename, int lines, int* lines_read, cpu_log_t* buffer) {
    FILE* fp = fopen(filename, "r");
    for (int l = 0; l < lines; l++) {
        cpu_log_t log;

        char* line = NULL;
        size_t len = 0;

        if (getline(&line, &len, fp) == -1) {
            break;
        }

        char* tok = strtok(line, " ");

        for (int r = 0; r < 16; r++) {
            log.r[r] = strtol(tok, NULL, 16);
            tok = strtok(NULL, " ");
        }
        tok = strtok(NULL, " "); // Skip ' cpsr: '

        log.cpsr.raw = strtol(tok, NULL, 16);

        buffer[l] = log;

        free(line);

        if (lines_read) {
            (*lines_read)++;
        }
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


int main(int argc, char** argv) {
    cflags_t* flags = cflags_init();

    const char* log_file = NULL;
    int log_lines = 0;
    word skip_to_address = 0x08008d08;
    bool should_skip_to_address = true;

    cflags_add_string(flags, 'f', "log-file", &log_file, "log file to check run against");
    cflags_add_int(flags, 'l', "num-log-lines", &log_lines, "number of lines in the file to check");

    cflags_parse(flags, argc, argv);

    const char* rom = flags->argv[0];

    if (flags->argc != 1 || log_file == NULL || log_lines == 0) {
        cflags_print_usage(flags,
                           "-f logfile -l numlines FILE",
                           "logtester, part of gba, a dgb gba emulator",
                           "https://github.com/Dillonb/gba");
        return 1;
    }

    log_set_verbosity(0);

    init_gbasystem(rom, NULL);

    set_register(cpu, 0, 0x00000000);
    set_register(cpu, 1, 0x00000000);
    set_register(cpu, 2, 0x00000000);
    set_register(cpu, 3, 0x00000000);
    set_register(cpu, 4, 0x00000000);
    set_register(cpu, 5, 0x00000000);
    set_register(cpu, 6, 0x00000000);
    set_register(cpu, 7, 0x00000000);
    set_register(cpu, 8, 0x00000000);
    set_register(cpu, 9, 0x00000000);
    set_register(cpu, 10, 0x00000000);
    set_register(cpu, 11, 0x00000000);
    set_register(cpu, 12, 0x00000000);
    set_register(cpu, REG_SP, 0x03007F00);
    cpu->sp     = 0x03007F00;
    cpu->sp_irq = 0x03007FA0;
    cpu->sp_svc = 0x03007FE0;
    set_register(cpu, REG_LR, 0x08000000);

    set_pc(cpu, 0x08000000);
    cpu->cpsr.raw = 0x0000001F;

    cpu_log_t* lines = malloc(sizeof(cpu_log_t) * log_lines);

    loginfo("Loading log")
    int log_lines_in_file = 0;
    load_log(log_file, log_lines, &log_lines_in_file, lines);

    if (log_lines > log_lines_in_file) {
        log_lines = log_lines_in_file;
    }

    loginfo("ROM loaded: %lu bytes", mem->rom_size)
    loginfo("Beginning CPU loop")
    int step = 0;

    printf("Skipping to correct start address if requested\n");

    while (should_skip_to_address && cpu->pc - (cpu->cpsr.thumb ? 2 : 4) != skip_to_address) {
        gba_system_step();
    }

    log_set_verbosity(4);
    loginfo("Starting trace at 0x%08X", cpu->pc - (cpu->cpsr.thumb ? 4 : 8))

    while(true) {
        if (step >= log_lines) {
            return 0; // We win!
        }
        word adjusted_pc = cpu->pc - (cpu->cpsr.thumb ? 4 : 8);
        // Register values in the log are BEFORE EXECUTING the instruction on that line
        logdebug("Checking registers (mode %d) against step %d (line %d in log)", cpu->cpsr.mode, step, step + 1)

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

        while(gba_dma() > 0) {
            loginfo("DMA in progress, please hold...")
        }

        do {
            gba_system_step();
        } while(cpu_stepped == false);

        step++;
    }
}
