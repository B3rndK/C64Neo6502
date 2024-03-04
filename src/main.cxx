/**
 * Install the RP2040 C++ SDK incl. compiling the debugger.
 * Create and switch to a build subdirectory, type "cmake .."
 * Type 'make' from the build directory.
 * On first IDE start, select the "active kit" using the bottom menu bar of vscode.
 * To debug, please remember to start openocd before in a shell.
 * Start debug output as well for debugging in minicom.
 * 
 * Written 2023, 2024 by Bernd Krekeler, Herne, Germany
 * 
 * 1- R/W#
 * 2- A0
 * 3- A1
 * 4- A2
 * 5- D0
 * 6- RESB#
*/

#include "stdinclude.hxx"

#define WAIT_FOR_USB_INIT_TIMEOUT_IN_MS 2000

int main() {
  adc_init();    
  stdio_init_all();
  sleep_ms(2000);
  /* In case of problems, RESET USB and exit in order to make ttyACM1 reappear. */
  //reset_usb_boot(0,0);  return 0; 

  Logging *pLog=new Logging(new AnsiTerminal(), All);
  pLog->Clear();
  pLog->LogTitle({"*** Welcome to the wonderful world of 8-bit MOS technology ***\n"});

  Computer *pComputer = new Computer(pLog);
  int ret=pComputer->Run();
  // Dead end...
  delete pLog;
  delete pComputer;
  sleep_ms(1000);
  return ret;
}

