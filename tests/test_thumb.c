#include <stdlib.h>
#include "test_common.h"

#define NUM_LOG_LINES 745
#define LOG_FILE "thumb.log"
#define TEST_FAILED_ADDRESS 0x0800092E

int main(int argc, char** argv) {
    exit(test_loop("thumb.gba", NUM_LOG_LINES, LOG_FILE, TEST_FAILED_ADDRESS));
}
