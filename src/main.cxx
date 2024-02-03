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
#include <pico/multicore.h>
#include <hardware/regs/resets.h>
#include <hardware/resets.h>
#include <hardware/vreg.h>
#include <pico/bootrom.h>
#include <memory.h>
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "terminalBase.hxx"
#include "ansiTerminal.hxx"
#include "logging.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"
#include <dvi.h>
#include <dvi_serialiser.h>
#include "hardware/structs/bus_ctrl.h"

#define WAIT_FOR_USB_INIT_TIMEOUT_IN_MS 2000
#define TURBO_65C02 252000 // 2,04 Mhz 65C02
#define REGULAR_65C02 133000 // 1,05 Mhz 65C02

dvi_inst dvi; 
uint8_t *pVideoMemory;

static const struct dvi_serialiser_cfg picodvi_cfg = {
   .pio = pio0,
   .sm_tmds = {0, 1, 2},
   .pins_tmds = {14, 18, 16},
   .pins_clk = 12,
   .invert_diffpairs = true
};


void initDVI(vreg_voltage voltage, dvi_inst *pDVI)
{
  vreg_set_voltage(voltage);                    				
  set_sys_clock_khz(TURBO_65C02, true); 
  pVideoMemory=(uint8_t *)calloc(320*240*2,sizeof(uint8_t));
  for (int y=0;y<240;y++)
  {
    for (int x=0;x<320;x++)
    {
      pVideoMemory[x*y]=x+y/x+1;
    }
  }

  dvi_init(pDVI,next_striped_spin_lock_num(), next_striped_spin_lock_num());
}

static void __not_in_flash_func(core1_main)() {
   dvi_register_irqs_this_core(&dvi, DMA_IRQ_1);   	// Enable IRQs
   dvi_start(&dvi);                         					// Start DVI library
   dvi_scanbuf_main_16bpp(&dvi);                  	 // State we are using 16 bit (e.g. 565) render
}

static void __not_in_flash_func(raceTheBeam)(void) {
   uint16_t *pScanLine;
   static int counter=0;
   while (queue_try_remove_u32(&dvi.q_colour_free, &pScanLine));
   pScanLine=(uint16_t *)(pVideoMemory+counter*320);
   counter++;
   queue_add_blocking_u32(&dvi.q_colour_valid, &pScanLine); 
   if (counter==240) counter=0;
}

int main() {
  adc_init();
  stdio_init_all();
  sleep_ms(WAIT_FOR_USB_INIT_TIMEOUT_IN_MS); // We need some time for USB to initialize.
  
  /* In case of problems, RESET USB and exit in order to make ttyACM1 reappear. 
  reset_usb_boot(0,0);  return 0; */

#ifdef _TURBO
  set_sys_clock_khz(TURBO_65C02, true); 
#else
  set_sys_clock_khz(REGULAR_65C02, true); 
#endif
  Logging *pLog=new Logging(new AnsiTerminal(), All);
  pLog->Clear();
  pLog->LogTitle({"*** Welcome to the wonderful world of 8-bit MOS technology ***\n"});
  
  memset(&dvi,0,sizeof(dvi));
  dvi.scanline_callback = raceTheBeam;
  dvi.timing=&dvi_timing_640x480p_60hz;
  dvi.ser_cfg = picodvi_cfg;
  initDVI(VREG_VOLTAGE_1_20, &dvi);
  multicore_launch_core1(core1_main);   
  
  Computer *pComputer = new Computer(pLog);
  
  int ret=pComputer->Run();
  delete pLog;
  delete pComputer;
  // Give some time to flush logging...
  sleep_ms(1000);
  return ret;
}

