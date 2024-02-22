# C64Neo6502
C64 emulation using the Neo6502 development board

## What it is used for
I coded this because the Neo6502 seems to be a very good piece of hardware to refresh old memories and to deal with microprocessor bus interfacing on hardware level by using a common high level programming language instead of e.g. VHDL or VERILOG. I see this as being in the middle of purely software based emulation and pure hardware simulation by using FPGAs. By looking at the code and debugging you can see how to initiate the RESET sequence of the 65C02, how to interface with RAM / ROM and how to do mapped IO and create hardware interrupts. It also enables to debug the internals of the C-64 to check, e.g. how the Kernal detects PAL or NTSC machines etc. To cut a long story short: It should be fun!

## What it is not
It is not an emulation which you can use to play any games (yet). If it was of any use to bring it further being a full emulation, use it. 

## Prerequisits
I am following the wireing standard from Vaselin Sladkov, so in order to have the RP2040 create real interrupts please connect pin 10 of UEXT connector (GPIO 25) to pin 24 of 6502 bus connector (IRQ) using an external wire. Furthermore we will need to create a real RESET signal, so please connect pin 9 of the UEXT connector to pin 40 (RESET) as well as pin 8 of the UEXT connector to pin 26 (NMI) of the 6502 bus.

### PICO-SDK (RP2040)
You need to install the PICO-SDK [Pico-SDK](https://github.com/raspberrypi/pico-sdk) and set the environment variable `PICO_SDK_PATH`. Furthermore I am using the Open On Chip Debugger (OpenOCD) using a [RP Debug Probe](https://www.raspberrypi.com/products/debug-probe/).

### tinyusb
I am using [tinyUSB](https://github.com/hathach/tinyusb) as well.

### PicoDVI
I make use of [PicoDVI](https://github.com/Wren6991/PicoDVI).

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

## Configuration
You can change the speed of the RP2040 by defining the macro "_TURBO". This will overclock your RP2040 from 133Mhz to 252MHz giving an effective C-64 speed of 2.04Mhz.

## Output
There is no DVI library used (yet). The output is simply a check if something is written to the C-64's video ram area and the current video ram is then sent to the console after doing a rudimentary character conversion. Idea is to later output the display via TCP/IP towards a WebGL browser window, but you can also take over the existing code to implement DVI output.

## Input
No keyboard input so far. Maybe I will implement it to at least run Commodore Basic programs but, as stated earlier, feel free to add it yourself with the usblib.

## WIP
This is work in progress and is set up for fun. I will continue to improve the emulation by adding keyboard support next, then DVI support.

## How does it look?
This is what will come out. I set a breakpoint to show the setup and then pressed F5 to continue running. The '@' signs occur because I fill the C-64's memory with 0 on startup:

https://github.com/B3rndK/C64Neo6502/assets/47975140/7b570d02-d5ab-46fd-90f3-c7ce108a266b



