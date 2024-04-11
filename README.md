# C64Neo6502
C64 emulation using the Neo6502 development board 

## What it is used for
I coded this because the Neo6502 seems to be a very good piece of hardware to refresh old memories and to deal with microprocessor bus interfacing on hardware level by using a common high level programming language instead of e.g. VHDL or VERILOG. I see this as being in the middle of purely software based emulation and pure hardware simulation by using FPGAs. By looking at the code and debugging you can see how to initiate the RESET sequence of the 65C02, how to interface with RAM / ROM and how to do mapped IO and create hardware interrupts. It also enables to debug the internals of the C-64 to check, e.g. how the Kernal detects PAL or NTSC machines etc. The real fun is to learn C-64 programming or even to extend this emulation. May the source code inspire other developers to reuse it and make it better. To cut a long story short: It should be fun!

## What it is not
It is an emulation which you can use to play around with, investigate, get more and more games runnig and even play a lot of commercial games. However, you have to dump game game roms and integrate them yourself. I cannot provide any roms to you, but you can easily copy from what is already there and what I have already done. For game ROMS and how to integrate, the same is valid like for the ROMs section below.

## Making gaming possible (for non-developers)
It is possible to integrate a whole bunch of working ROMs and have them all stored in the RP2040 flash simultaneously. Using a small C64 (yes, 6502) program ... you could offer a game selection menu on startup. When a game is selected, the program could, for example, poke a magic value identifying the game into an unused IO area. The emulator would then see that a value is stored in this "IO register" and could then load and run the game from flash.

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
Building this emulator is straightforward. Create (mkdir) and then cd to a **build** subfolder, then run `cmake ..`
Please see the `CMakeLists.txt` file in case you do not want Simon's Basic or monitor support. You can simply remove `_SIMONS_BASIC` from the compile definition list.  

## ROMs
Due to copyright reasons, I cannot include the C-64 bios files "basic", "kernal" and "chargen". So please use e.g. the tool "bin2hdr" from Veselin Sladkov [Reload-Emulator](https://github.com/vsladkov/reload-emulator)) to convert your C-64 rom files to 

<ul>
  <li>basic.hxx</li>
  <li>kernal.hxx</li>
  <li>chargen.hxx</li>
</ul><br>
replacing the included but invalid files.
<BR>
<BR>
Note: In case you want to upload the compiled `UF2` of this emulator to a website, please ensure you are legally entitled to do this because of these still copyrighted ROMs.

### Simon's Basic
This was a very popular expanded basic for the C-64. I also included support for this. You need to dump $8000-BFFF of the software. I cannot provide it to you due to copyright laws:
<ul>
  <li>simons_basic.hxx</li>
</ul><br>
In case you have licensed Simon's Basic, you can enable this module within the `CMakeLists.txt`. It is switched off by default.
IPXL_20240411_123345109.TS (1)n order to boot Simon's Basic, you have to enter `POKE 32776,48` and then restart the C-64 by typing `SYS 64738`. You will be presented the welcome screen. Here you can type `OLD` to recover the basic program. Then enter `LIST` to edit it or `RUN` to start it.

### Monitor
I included a small machine code monitor at $c000 (`SYS 49152`).
You can enable this module within the `CMakeLists.txt`. It is switched off by default.

## Sound
Source Code of TinySid is now included but it is WIP. NightShade sounds quite well while others, hmmm... ok...

## Output
DVI output is now implemented for all official C-64 VIC modes, textmode, multicolor textmode, hires, hires multicolor and extended color mode (ECM) . The design also supports fli support. No support for sprites or bitscrolling yet. The resolution used is a "quirk mode" of 340x240 and may not run on every display. You can enforce using a 640x480 mode by changing a single line of code in case you prefer a more safe timing.

## Input
Keyboard input is currently handled by directly attaching a usb keyboard. There is currently no explicit USB-hub, so you have to connect your USB keyboard either directly or use a working USB hub. I am using the keyboard of the RaspberryPi foundation. Joystick supported in Port A (SNES_OEM type).

## WIP
This is work in progress and is set up for fun. 

## How does it look?
* This is how it currently looks like:

https://github.com/B3rndK/C64Neo6502/assets/47975140/b9c2fc19-d006-4a64-adb6-9a790b7ab940

* How to activate the expanded basic (and run a little 3D-Mandelbrot-Set):


https://github.com/B3rndK/C64Neo6502/assets/47975140/3b8debd9-2552-4bcd-9083-cc5b92138464

* And the final result after some hours of computation...
  
![result](https://github.com/B3rndK/C64Neo6502/assets/47975140/7d0ea53a-ef3a-47bd-9427-aa0bf179def9)


Some of the games tested:


https://github.com/B3rndK/C64Neo6502/assets/47975140/af08d204-2eb9-44d3-87b7-aa33b6138d24


https://github.com/B3rndK/C64Neo6502/assets/47975140/61c79733-661e-4234-8802-bc1f019b2992


https://github.com/B3rndK/C64Neo6502/assets/47975140/1d2e7dc5-e1bf-4e9b-a485-95c238789b21


https://github.com/B3rndK/C64Neo6502/assets/47975140/77913075-94d3-4c06-aabf-6d9f935e963c


