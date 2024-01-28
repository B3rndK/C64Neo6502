cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)
#set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project 

add_executable(computer
    main.cxx
    logging.cxx
    rp65c02.cxx 
    vic6569.cxx
    cia6526.cxx
    rpPetra.cxx
    computer.cxx
    
)

# Comment in for release version 
add_compile_definitions(_DEBUG _NO_TURBO) 

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(computer pico_stdlib hardware_adc pico_time)

# enable usb output, disable uart output
pico_enable_stdio_usb(computer 1)
pico_enable_stdio_uart(computer 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(computer)