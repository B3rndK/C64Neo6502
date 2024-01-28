/**
 * Install the RP2040 C++ SDK incl. compiling the debugger.
 * Create and switch to a build subdirectory, type "cmake .."
 * Type 'make' from the build directory.
 * On first IDE start, select the "active kit" using the bottom menu bar of vscode.
 * To debug, please remember to start openocd before in a shell.
 * Start debug output as well for debugging in minicom.
 * 
 * (C)2023 Bernd Krekeler, Herne, Germany
 * 
 * 1- R/W#
 * 2- A0
 * 3- A1
 * 4- A2
 * 5- D0
 * 6- RESB#
*/

#include <stdio.h>
#include <hardware/adc.h>
#include <pico/stdlib.h>
#include <hardware/regs/resets.h>
#include <hardware/resets.h>
#include <hardware/vreg.h>
#include <pico/bootrom.h>
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "terminalBase.hxx"
#include "ansiTerminal.hxx"
#include "logging.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"

#define WAIT_FOR_USB_INIT_TIMEOUT_IN_MS 2000
#define TURBO_65C02 252000 // 2,04 Mhz 65C02
#define REGULAR_65C02 133000 // 1,05 Mhz 65C02

int main() {
  adc_init();
  stdio_init_all();
  sleep_ms(WAIT_FOR_USB_INIT_TIMEOUT_IN_MS); // We need some time for USB to initialize.
  //reset_usb_boot(0,0);  
#ifdef _TURBO
  set_sys_clock_khz(TURBO_65C02, true); 
#else
  set_sys_clock_khz(REGULAR_65C02, true); 
#endif
  Logging *pLog=new Logging(new AnsiTerminal(), All);
  pLog->Clear();
  pLog->LogTitle({"*** Welcome to the wonderful world of 8-bit MOS Technology ***\n"});
  Computer *pComputer = new Computer(pLog);
  
  int ret=pComputer->Run();
  delete pLog;
  delete pComputer;
  sleep_ms(1000);
  // reset_usb_boot(0,0);  
  return 0;
} 