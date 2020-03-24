SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(SDL2 REQUIRED)

INCLUDE_DIRECTORIES(SYSTEM "contrib" ${SDL2_INCLUDE_DIR})

add_library(render
        render.c render.h)

target_link_libraries(render ${SDL2_LIBRARY})

add_library(core
        gbabios.c gbabios.h
        gbabus.c gbabus.h
        gbarom.c gbarom.h
        gbamem.c gbamem.h
        ppu.c ppu.h
        render.c render.h
        ioreg_util.h)

add_subdirectory(arm7tdmi)
add_subdirectory(common)

add_executable (gba gba.c)
target_link_libraries(gba common arm7tdmi core render)

add_executable (logtester logtester.c ../tests/test_common.h)
target_link_libraries(logtester common arm7tdmi core render)
