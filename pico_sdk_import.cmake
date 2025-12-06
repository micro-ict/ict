# Adjust this path to where you cloned pico-sdk
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

set(PICO_PLATFORM, "rp2350-arm-s")
set(PICO_BOARD_HEADER_DIR "${CMAKE_CURRENT_LIST_DIR}/boards")
set(PICO_BOARD "pico2")
set(PICO_COPMILER, "pico_arm_cortex_m33_gcc")
set(PICO_GCC_TRIPLE, "pico-arm-none-eabi")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(CMAKE_C_FLAGS "-mcpu=cortex-m33 -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-mcpu=cortex-m33 -mthumb -O3 -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

# Import the Pico SDK's core setup
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)