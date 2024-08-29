/**
 * Clone the pico-sdk first and set the PICO_SDK_PATH environment variable (.bashrc).
 * Create and change to the C64Neo6502 build subdirectory. Type "cmake ../", then "make".
 * Debugging using OpenOCD: You must compile the https://github.com/raspberrypi/openocd due to the
 * fact that the Neo6502 is using a flash memory the standard openocd package 12.0 currently does
 * not support. Check the OpenOCD git page how to compile. Important: 
 * when configuring, please use "./configure --enable-cmsis-dap-v2 --enable-cmsis-dap --enable-openjtag"
 * Note: You may be asked install additional libraries before. Do not forget to run "sudo make --install".
 *    ./bootstrap
 *    . /configure --enable-cmsis-dap --enable-cmsis-dap-v2 --enable-openjtag
 *    ./make
 *    ./make install
 * Install GNU-Debugger multi-architecture: sudo apt install gdb-multiarch
 * Install VS Code CMake/Tools/C++ extensions and Cortex-Debug
 * Run and Debug: Select "Cortex Debug" 
 * Type 'make' from the build directory.
 * On first IDE start, select the "active kit" (Ctrl+Shift+P: Select a Kit)
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
/* ELITE
CIA-B write access: dd0d, 0003
CIA-B write access: dd02, 003f
CIA-B write access: dd00, 0016
CIA-B write access: dd00, 0006

CIA-A write access: dc04, 0037
CIA-A write access: dc05, 0064
CIA-A write access: dc0d, 0129
CIA-A write access: dc0e, 0017
CIA-B write access: dd00, 0017

*/

int main() {
  adc_init();    
  stdio_init_all();
  // sleep_ms(3000);
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

