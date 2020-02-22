SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(SYSTEM "contrib")

add_library(core
        rom.c
        rom.h
        )

add_executable (gba gba.c)
target_link_libraries(gba core)