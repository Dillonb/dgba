SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(SDL2 REQUIRED)
IF(NOT MinGW)
find_package(Capstone)
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM "contrib" ${SDL2_INCLUDE_DIR})

add_library(render
        graphics/render.c graphics/render.h)

IF(MinGW)
    target_link_libraries(render -lSDL2)
ELSE()
    target_link_libraries(render ${SDL2_LIBRARY})
ENDIF()

add_library(core
        gba_system.c gba_system.h
        mem/gbabios.c mem/gbabios.h
        mem/gbabus.c mem/gbabus.h
        mem/gbarom.c mem/gbarom.h
        mem/gbamem.c mem/gbamem.h
        graphics/ppu.c graphics/ppu.h
        graphics/render.c graphics/render.h
        graphics/debug.c graphics/debug.h
        mem/dma.c mem/dma.h
        disassemble.c disassemble.h
        mem/ioreg_util.h mem/ioreg_names.h
        mem/backup/flash.c mem/backup/flash.h
        mem/gpio/gpio.c mem/gpio/gpio.h
        mem/gpio/rtc.c mem/gpio/rtc.h
        mem/mgba_debug.c mem/mgba_debug.h
        mem/backup/eeprom.c mem/backup/eeprom.h)
target_link_libraries(core m)

IF(NOT MinGW)
    IF(Capstone_FOUND)
        TARGET_LINK_LIBRARIES(core Capstone::Capstone)
        TARGET_COMPILE_DEFINITIONS(core PRIVATE -DHAVE_CAPSTONE)
    ENDIF()
ENDIF()

add_subdirectory(arm7tdmi)
add_subdirectory(audio)
add_subdirectory(common)

add_executable (${GBA_TARGET} gba.c gba_system.c gba_system.h)
target_link_libraries(${GBA_TARGET} common arm7tdmi core audio render)

IF(NOT MinGW)
add_executable (logtester logtester.c gba_system.c gba_system.h)
target_link_libraries(logtester common arm7tdmi core audio render)
ENDIF()
