#include <stdlib.h>
#include "test_common.h"

#define TEST_FAILED_ADDRESS 0x08001B94
#define WATCH_REG 12
#define NUM_LOG_LINES 1107
#define LOG_FILE "arm.log"

int main(int argc, char** argv) {
    exit(test_loop("arm.gba", NUM_LOG_LINES, LOG_FILE, TEST_FAILED_ADDRESS, WATCH_REG));
}
