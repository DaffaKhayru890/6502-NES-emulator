add_executable(tinynes_tests
    test_main.c
    test_helpers.c
    cpu/test_cpu_registers.c
    cpu/test_cpu_addressing.c
    cpu/test_cpu_instructions.c
    bus/test_cpu_bus.c
    ppu/test_ppu_memory.c
    ppu/test_ppu_registers.c
    cartridge/test_nrom.c
    apu/test_apu.c
    integration/test_nes_integration.c
)

target_include_directories(tinynes_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tinynes_tests PRIVATE tinynes_core)

add_test(NAME tinynes_unit COMMAND tinynes_tests)
