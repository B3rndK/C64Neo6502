# C64Neo6502
C64 emulation using the Neo6502 development board

## What it is used for
I coded this because the Neo6502 seemed to be a very good piece of hardware allowing to get known to microprocessor bus interfacing on hardware level but by using a common high level programming language. I see this as being in the middle of either software based emulation and pure hardware emulation using FPGAs. By looking at the code you can see how to initiate the RESET sequence of the 65C02, how to interface with RAM / ROM and how to do mapped IO and create hardware interrupts. It also enables to debug the internals of the C-64 to check, e.g. how the Kernal detects PAL or NTSC machines etc. 

## What it is not
It is not an emulation which you can use to play any games. If it was of any use to bring it further being a full emulation, I would be happy.

## Prerequisits
I am following the wireing standard from Vaselin Sladkov, so in order to have the RP2040 create real interrupts please connect pin 10 of UEXT connector (GPIO 25) to pin 24 of 6502 bus connector (IRQ) using an external wire. Furthermore we will need to create a real RESET signal, so please connect pin 9 of the UEXT connector to pin 40 (RESET) as well as pin 8 of the UEXT connector to pin 26 (NMI) of the 6502 bus.

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

# Output
There is no DVI library used. The output is simply a check if something is written to the C-64's video ram area and the current video ram is then sent to the console after doing a rudimentary character conversion. Idea is to later output the display via TCP/IP towards a WebGL browser window, but you can also take over the existing code to implement DVI output.

# Input
No keyboard input so far. Maybe I will implement it to at least run Commodore Basic programs but, as stated earlier, feel free to add it yourself with the usblib.

