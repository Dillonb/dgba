#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define COLOR_RED       "\033[0;31m"
#define COLOR_YELLOW    "\033[0;33m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_END       "\033[0;m"


int log_get_verbosity();
void log_set_verbosity(unsigned int level);

// TODO make these actually respect the verbosity
#define logfatal(message,...) if (1) {errx(EXIT_FAILURE, COLOR_RED message "\n" COLOR_END, ##__VA_ARGS__);}
#define logwarn(message,...) if (log_get_verbosity() >= 0) {printf(COLOR_YELLOW message "\n" COLOR_END, ##__VA_ARGS__);}
#define loginfo(message,...) if (log_get_verbosity() >= 0) {printf(COLOR_CYAN message "\n" COLOR_END, ##__VA_ARGS__);}
#define logdebug(message,...) if (log_get_verbosity() >= 0) {printf(COLOR_CYAN message "\n" COLOR_END, ##__VA_ARGS__);}
#endif