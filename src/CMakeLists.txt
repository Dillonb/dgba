SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(SYSTEM "contrib")

add_library(core
        util.h
        gbabus.c
        gbabus.h
        gbarom.c
        gbarom.h
        log.c
        log.h
        gbamem.c
        gbamem.h
        arm7tdmi.c
        arm7tdmi.h
        arm_instr.h
        ioreg_util.h
        )

add_executable (gba gba.c)
target_link_libraries(gba core)
