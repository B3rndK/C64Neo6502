# C64Neo6502
C64 emulation using the Neo6502 development board

## What it is used for
I coded this because the Neo6502 seems to be a very good piece of hardware to refresh old memories and to deal with microprocessor bus interfacing on hardware level by using a common high level programming language instead of e.g. VHDL or VERILOG. I see this as being in the middle of purely software based emulation and pure hardware simulation by using FPGAs. By looking at the code and debugging you can see how to initiate the RESET sequence of the 65C02, how to interface with RAM / ROM and how to do mapped IO and create hardware interrupts. It also enables to debug the internals of the C-64 to check, e.g. how the Kernal detects PAL or NTSC machines etc. To cut a long story short: It should be fun!

## What it is not
It is not an emulation which you can use to play serious games (yet). The real fun is to learn and to extend this emulation. May the source code inspire other developers to reuse it and make it better.

## Prerequisits
I am following the wireing standard from Vaselin Sladkov, so in order to have the RP2040 create real interrupts please connect pin 10 of UEXT connector (GPIO 25) to pin 24 of 6502 bus connector (IRQ) using an external wire. Furthermore we will need to create a real RESET signal, so please connect pin 9 of the UEXT connector to pin 40 (RESET) as well as pin 8 of the UEXT connector to pin 26 (NMI) of the 6502 bus.

### PICO-SDK (RP2040)
You need to install the PICO-SDK [Pico-SDK](https://github.com/raspberrypi/pico-sdk) and set the environment variable `PICO_SDK_PATH`. Furthermore I am using the Open On Chip Debugger (OpenOCD) using a [RP Debug Probe](https://www.raspberrypi.com/products/debug-probe/).

### tinyusb
https://github.com/hathach/tinyusb

### PicoDVI
https://github.com/Wren6991/PicoDVI/tree/master/software

### Visual Studio Code
I am using Visual Studio Code and the following plugins:

<ul>
<li>C/C++ (incl. Extension Pack and Themes)</li>
<li>CMake</li>
<li>CMake Tools</li>
<li>Cortex Debug</li>
</ul>

### Build
Create and cd a **build** subfolder, then run **cmake ..*

## ROMs
Due to copyright reasons, I cannot include the C-64 bios files "basic", "kernal" and "chargen". So please use e.g. the tool "bin2hdr" from Veselin Sladkov [Reload-Emulator](https://github.com/vsladkov/reload-emulator)) to convert your C-64 rom files to 
<ul>
  <li>basic.hxx</li>
  <li>kernal.hxx</li>
  <li>chargen.hxx</li>
</ul><br>
replacing the included but invalid files.

### Monitor
I included a small machine code monitor at $c000 (sys 49152).

## Output
DVI output is now implemented for C64 textmode. The design also supports hires and even multicolour (fli) support. 
It is using a "quirk mode" of 340x240 and may not run on every monitor. You can enforce using a 640x480 mode by changing a single line of code.

## Input
Keyboard input is currently handled by directly attaching a usb keyboard. There is currently no USB-hub support.

## WIP
This is work in progress and is set up for fun. I will continue to improve the emulation by adding hires-mode (320x200) and better keyboard support via CIA#1 next.

## How does it look?
This is what will come out. I set a breakpoint to show the setup and then pressed F5 to continue running. The '@' signs occur because I fill the C-64's memory with 0 on startup:

https://github.com/B3rndK/C64Neo6502/assets/47975140/7b570d02-d5ab-46fd-90f3-c7ce108a266b



