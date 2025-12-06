cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
include(pico_sdk_import.cmake)

project(pico_ict C CXX ASM)

pico_sdk_init()

add_executable(pico_ict
    main.cpp
)

target_link_libraries(pico_ict
    pico_stdlib
    pico_rand
    hardware_uart
    hardware_dma
    hardware_irq
)

pico_set_linker_script(pico_ict ${CMAKE_CURRENT_LIST_DIR}/mem.ld)
target_compile_definitions(pico_ict PRIVATE PICO_USE_PSRAM=1)

pico_add_extra_outputs(pico_ict)

pico_enable_stdio_uart(pico_ict 0)
pico_enable_stdio_usb(pico_ict 1)
