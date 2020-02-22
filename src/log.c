#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <err.h>

#include "log.h"

#define COLOR_RED       "\033[0;31m"
#define COLOR_YELLOW    "\033[0;33m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_END       "\033[0;m"

unsigned int verbosity = 0;



void LOG_set_verbosity(unsigned int new_verbosity) {
    verbosity = new_verbosity;
}

void LOG(LOGLEVEL level, char* message, ...) {
    va_list args;
    va_start(args, message);
    switch (level) {
        case FATAL:
            printf(COLOR_RED);
            errx(EXIT_FAILURE, message, args);
        case ERROR:
            printf("%s[ERROR] ", COLOR_RED);
            break;
        case WARN:
            printf("%s[WARN] ", COLOR_YELLOW);
            break;
        case INFO:
            printf("%s[INFO] ", COLOR_CYAN);
            break;
        case DEBUG:
            printf("%s[DEBUG] ", COLOR_CYAN);
            break;
        default:
            errx(EXIT_FAILURE, "Unknown LOGLEVEL");
    }
    vprintf(message, args);
    printf("%s\n", COLOR_END);
    va_end(args);
}
