SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(SDL2 REQUIRED)

INCLUDE_DIRECTORIES(SYSTEM "contrib" ${SDL2_INCLUDE_DIR})

add_library(render
        graphics/render.c graphics/render.h)

target_link_libraries(render ${SDL2_LIBRARY})

add_library(core
        mem/gbabios.c mem/gbabios.h
        mem/gbabus.c mem/gbabus.h
        mem/gbarom.c mem/gbarom.h
        mem/gbamem.c mem/gbamem.h
        graphics/ppu.c graphics/ppu.h
        graphics/render.c graphics/render.h
        graphics/debug.c graphics/debug.h
        mem/ioreg_util.h)

add_subdirectory(arm7tdmi)
add_subdirectory(common)

add_executable (gba gba.c gba_system.c gba_system.h)
target_link_libraries(gba common arm7tdmi core render)

add_executable (logtester logtester.c ../tests/test_common.h)
target_link_libraries(logtester common arm7tdmi core render)
