SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

INCLUDE_DIRECTORIES(SYSTEM "contrib")

add_library(core
        gbabus.c
        gbabus.h
        gbarom.c
        gbarom.h
        gbamem.c
        gbamem.h
        ioreg_util.h
        )

add_subdirectory(arm7tdmi)
add_subdirectory(common)

add_executable (gba gba.c)
target_link_libraries(gba common arm7tdmi core)
