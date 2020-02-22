#ifndef __LOG_H__
#define __LOG_H__
typedef enum LOGLEVEL_t {
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG
} LOGLEVEL;

void LOG(LOGLEVEL level, char* message, ...);
void LOG_set_verbosity(unsigned int level);
#endif