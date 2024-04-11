cmake_minimum_required(VERSION 3.27.4...4.0)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_BOARD pico)
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)
#set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

include(FetchContent)

FetchContent_Declare(
    tinyusb
    GIT_REPOSITORY https://github.com/hathach/tinyusb.git
    GIT_TAG        0.16.0
)
FetchContent_MakeAvailable(tinyusb)
set(PICO_TINYUSB_PATH ${tinyusb_SOURCE_DIR})

pico_sdk_init()

add_compile_options(-Wall -Werror -g -O2)

# To change the default serial config for all apps, pass e.g.
#   cmake -DDVI_DEFAULT_SERIAL_CONFIG=pimoroni_demo_hdmi_cfg ..
# then rebuild. This is applied via target_compile_definitions on each app
# individually.
set(DVI_DEFAULT_SERIAL_CONFIG "pico_sock_cfg" CACHE STRING
    "Select a default pin configuration from common_dvi_pin_configs.h")

FetchContent_Declare(
    PicoDVI
    GIT_REPOSITORY https://github.com/Wren6991/PicoDVI
    GIT_TAG        a248d72b8cc95a08b43ef4307cd954eb801272b2 # just the latest on 11/12/2023
)
FetchContent_MakeAvailable(PicoDVI)

add_subdirectory(${picodvi_SOURCE_DIR}/software/libdvi ${picodvi_BINARY_DIR})

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}

  #  include
  #  common/include
  #  ${picodvi_SOURCE_DIR}/software/libdvi
  #  ${picodvi_SOURCE_DIR}/software/libdvi    
  #  ${tinyusb_SOURCE_DIR}
  #  ${tinyusb_SOURCE_DIR}/src/hw/rp2040
)

# rest of your project 
add_executable(computer
  videoOut.cxx
  main.cxx
  logging.cxx
  rp65c02.cxx 
  vic6569.cxx
  cia6526.cxx
  cia1.cxx
  cia2.cxx
  sid/sid.cpp
  rpPetra.cxx
  computer.cxx
  joysticks.cxx
  snes.cxx
  competitionPro.cxx
  keyboard.cxx
)

# Comment in for release version 
# _NMISTART => Start ROM using the Restore Key (F7)
add_compile_definitions(_DEBUG _NO_SID _NO_RASTERIRQ _NO_MONITOR_CARTRIDGE _NO_SIMONS_BASIC _NO_TRAPDOOR _NO_NIGHTSHADE _NO_ELITE _NO_PULSAR7 _NO_MERCENARY _NO_FAIRLIGHT _NO_FLASHDANCE _NO_WIZBALL _NO_NMISTART _NO_SYNTH_SAMPLE)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(computer pico_stdlib hardware_adc hardware_dma hardware_pio pico_time pico_multicore libdvi tinyusb_host tinyusb_board tinyusb_device)

# enable usb output, disable uart output
pico_enable_stdio_semihosting(computer 1)
pico_enable_stdio_usb(computer 0)
pico_enable_stdio_uart(computer 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(computer)