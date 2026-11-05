cmake_minimum_required(VERSION 3.16)

project(TinyNES
    VERSION 0.1.0
    DESCRIPTION "Emulator Nintendo Entertainment System modular dalam C17"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Tipe build" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

set(TINYNES_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
)

option(TINYNES_SANITIZE "Aktifkan AddressSanitizer dan UBSan" OFF)
option(TINYNES_BUILD_TESTS "Bangun unit test" ON)

# ---------------------------------------------------------------------------
# Inti emulator: sama sekali tidak bergantung pada SDL2.
# ---------------------------------------------------------------------------
add_library(tinynes_core STATIC
    src/core/nes.c
    src/core/config.c
    src/core/error.c

    src/cpu/cpu_2a03.c
    src/cpu/cpu_addressing.c
    src/cpu/cpu_instructions.c
    src/cpu/cpu_flags.c

    src/bus/cpu_bus.c

    src/ppu/ppu.c
    src/ppu/ppu_memory.c
    src/ppu/ppu_registers.c
    src/ppu/ppu_render.c
    src/ppu/ppu_background.c
    src/ppu/ppu_sprites.c
    src/ppu/ppu_palette.c

    src/apu/apu.c
    src/apu/apu_pulse.c
    src/apu/apu_triangle.c
    src/apu/apu_noise.c
    src/apu/apu_dmc.c
    src/apu/apu_frame_counter.c
    src/apu/apu_mixer.c

    src/cartridge/cartridge.c
    src/cartridge/ines_loader.c
    src/cartridge/mapper.c
    src/cartridge/mapper_nrom.c

    src/input/controller.c

    src/util/log.c
    src/util/file.c
)

target_include_directories(tinynes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(tinynes_core PRIVATE ${TINYNES_WARNINGS})

# Header internal (tidak dipublikasikan) tetap terlihat oleh modul inti.
target_include_directories(tinynes_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---------------------------------------------------------------------------
# Frontend SDL2: bergantung pada inti, bukan sebaliknya.
# ---------------------------------------------------------------------------
add_library(tinynes_frontend STATIC
    src/frontend/sdl_app.c
    src/frontend/sdl_video.c
    src/frontend/sdl_audio.c
    src/frontend/sdl_input.c
)

target_include_directories(tinynes_frontend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(tinynes_frontend PRIVATE ${TINYNES_WARNINGS})
target_link_libraries(tinynes_frontend PUBLIC PkgConfig::SDL2 tinynes_core)

# ---------------------------------------------------------------------------
add_executable(tinynes src/main.c)
target_compile_options(tinynes PRIVATE ${TINYNES_WARNINGS})
target_link_libraries(tinynes PRIVATE tinynes_frontend)

if(TINYNES_SANITIZE)
    foreach(target tinynes_core tinynes_frontend tinynes)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

if(TINYNES_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    if(TINYNES_SANITIZE)
        target_compile_options(tinynes_tests PRIVATE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(tinynes_tests PRIVATE -fsanitize=address,undefined)
    endif()
endif()

message(STATUS "TinyNES ${PROJECT_VERSION} - SDL2 ${SDL2_VERSION}")
