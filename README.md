# C64Neo6502
C64 emulation using the Neo6502 development board

# Prerequisits
I am following the wireing standard from Vaselin Sladkov, so in order to have the RP2040 create real interrupts please connect pin 10 of UEXT connector (GPIO 25) to pin 24 of 6502 bus connector (IRQ) using an external wire. Furthermore we will need to create a real RESET signal, so please connect pin 9 of the UEXT connector to pin 40 (RESET) as well as pin 8 of the UEXT connector to pin 26 (NMI) of the 6502 bus.

# ROMs
Due to copyright reasons, I cannot include the C-64 bios files "basic", "kernal" and "chargen". So please use the included tool "bin2hdr" from Veselin Sladkov to convert your C-64 rom files to 
<ul>
  <li>basic.hxx</li>
  <li>kernal.hxx</li>
  <li>chargen.hxx</li>
</ul>

  

