# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "C:/Users/ronal/OneDrive/Documentos/Embarcatechpico1/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(SemafaroTask C CXX ASM)
include_directories(${CMAKE_SOURCE_DIR}/lib)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(SemafaroTask SemafaroTask.c                 
             lib/rgb.c
             lib/ssd1306.c
             lib/buttons.c
             lib/display_init.c
             lib/matrixws.c
             lib/buzzer.c)

pico_set_program_name(SemafaroTask "SemafaroTask")
pico_set_program_version(SemafaroTask "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(SemafaroTask 0)
pico_enable_stdio_usb(SemafaroTask 1)

# Add the standard library to the build
target_link_libraries(SemafaroTask
        pico_stdlib
        hardware_pwm
        hardware_i2c
        hardware_adc
        hardware_pio
        hardware_gpio
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4)

# Add the standard include files to the build
target_include_directories(SemafaroTask PRIVATE
        ${CMAKE_SOURCE_DIR}
)

# Add any user requested libraries
target_link_libraries(SemafaroTask 
        
        )

pico_add_extra_outputs(SemafaroTask)

