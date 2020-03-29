#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>

#define COLOR_RED       "\033[0;31m"
#define COLOR_YELLOW    "\033[0;33m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_END       "\033[0;m"

#define LOG_VERBOSITY_WARN 1
#define LOG_VERBOSITY_INFO 2
#define LOG_VERBOSITY_DEBUG 3


int log_get_verbosity();
void log_set_verbosity(unsigned int level);

#define logfatal(message,...) if (1) { \
    fprintf(stderr, COLOR_RED "[FATAL] at %s:%d ", __FILE__, __LINE__);\
    fprintf(stderr, message "\n" COLOR_END, ##__VA_ARGS__);\
    exit(EXIT_FAILURE);}

#define logwarn(message,...) if (log_get_verbosity() >= LOG_VERBOSITY_WARN) {printf(COLOR_YELLOW "[WARN]  " message "\n" COLOR_END, ##__VA_ARGS__);}
#define loginfo(message,...) if (log_get_verbosity() >= LOG_VERBOSITY_INFO) {printf(COLOR_CYAN "[INFO]  " message "\n" COLOR_END, ##__VA_ARGS__);}
#define logdebug(message,...) if (log_get_verbosity() >= LOG_VERBOSITY_DEBUG) {printf("[DEBUG] " message "\n", ##__VA_ARGS__);}

#define unimplemented(condition, message) if (condition) { logfatal("UNIMPLEMENTED CASE DETECTED: %s", message) }
#endif
