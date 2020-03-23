#include <cflags.h>
#include "../tests/test_common.h"

int main(int argc, char** argv) {
    cflags_t* flags = cflags_init();

    const char* log_file = NULL;
    int log_lines = 0;

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

    test_loop(rom, log_lines, log_file, 0);
}
