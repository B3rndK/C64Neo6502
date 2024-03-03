/**
 * Generic video output for 340x240 resolution.
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

#include <stdlib.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <dvi.h>
#include <hardware/vreg.h>
#include <memory.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "videoOut.hxx"
#include "rpPetra.hxx"

const int LAST_FRAMEBUFFER_LINE=240;

// Timing for a generic 340x240 resolution (680x480, twin pixel)
const struct dvi_timing dvi_timing_340x240p_60hz = {

	.h_sync_polarity   = false,
	.h_front_porch     = 16,
	.h_sync_width      = 96,
	.h_back_porch      = 56, 
	.h_active_pixels   = 680,
	.v_sync_polarity   = false,
	.v_front_porch     = 10,
	.v_sync_width      = 2,
	.v_back_porch      = 33, 
	.v_active_lines    = 480,
	.bit_clk_khz       = 256000 
};

static const struct dvi_serialiser_cfg picodvi_cfg = {
   .pio = pio0,
   .sm_tmds = {0, 1, 2},
   .pins_tmds = {14, 18, 16},
   .pins_clk = 12,
   .invert_diffpairs = true,
   .prog_offs=0
};

// C64 color schema as 565
const uint16_t colorIndex[]={0x0000,0xffff,0x8187,0x7679,0x89f2,0x55e9,0x2973,0xef8e,0x9a85,0x51c0,0xC36E,0x4228,0x8c51,0x8ff1,0x8c5f,0xce59};

uint8_t *frameBuffer;
dvi_inst *g_pDVI; 
uint16_t *pScanLine;
uint16_t *pCurScanLine;

RpPetra *_pGlue; 

VideoOut::VideoOut(Logging *pLog, RpPetra *pGlue, uint8_t *pFrameBuffer)
{
   m_pLog=pLog;
   _pGlue=pGlue;
   frameBuffer=pFrameBuffer;
   pScanLine=(uint16_t *)calloc(680+32,sizeof(uint16_t));
}

static void __not_in_flash_func(core1_main)() {
   dvi_register_irqs_this_core(g_pDVI, DMA_IRQ_1);   
   dvi_start(g_pDVI);                       					
   dvi_scanbuf_main_16bpp(g_pDVI);  
}

/** Calculates a single scanline for DVI. We may switch to use the PIO coprocessor later on. */
static void __not_in_flash_func(beamRace)(void) 
{
  static int currentBeamPos=0;
  static int upperBorderStop;
  static int lowerBorderStart;
  static int leftBorderEnd;

  // Border handling
  if (_pGlue->m_pVICII->m_registerSetRead[0x11] & 0b00001000)  // Display is 25 lines
  {
    upperBorderStop=12;
    lowerBorderStart=220;
  }
  else  // 24 lines
  {
    upperBorderStop=20;
    lowerBorderStart=212;
  }
  if (_pGlue->m_pVICII->m_registerSetRead[0x16] & 0b00001000) // 40 cols
  {
    leftBorderEnd=20;
  }
  else 
  { // 38 cols
    leftBorderEnd=28;
  }
  bool isScreenSwitchedOff=((currentBeamPos<=upperBorderStop) || (currentBeamPos>=lowerBorderStart) || !(_pGlue->m_pVICII->m_registerSetRead[0x11] & 0x10));
  
  uint16_t color=colorIndex[(_pGlue->m_pVICII->m_registerSetRead[0x20]) & 0x0f];
  uint32_t *pP=(uint32_t *)pScanLine;
  
  // Draw the border(s), 32-bits at a time
  for (int i=0;i<340/2;i++)
  {
    pP[i]=(uint32_t) (color << 16) | color;
  }

  while (queue_try_remove_u32(&g_pDVI->q_colour_free, &pScanLine));  
  
  if (!isScreenSwitchedOff)
  {
    if (currentBeamPos<lowerBorderStart-upperBorderStop && currentBeamPos>upperBorderStop)
    {
      int x=leftBorderEnd/3;
      uint8_t *pCurBuffer = frameBuffer+((currentBeamPos-upperBorderStop)*160);
      uint8_t pixel; 
      // 2 pixels encoded in 4-bits... This way we can easily support even VIC's FLI color modes later on...
      for (int i=0;i<320/2;i++) {
          pixel=pCurBuffer[i]; 
          pScanLine[x++]=colorIndex[pixel>>4];
          pScanLine[x++]=colorIndex[pixel & 15];
      }
    }
  }
  queue_add_blocking_u32(&g_pDVI->q_colour_valid, &pScanLine); 

  if (++currentBeamPos==LAST_FRAMEBUFFER_LINE)
  {
    currentBeamPos=0;
  }
}

void VideoOut::Reset()
{
  g_pDVI=(dvi_inst *)calloc(1,sizeof(dvi_inst));
  g_pDVI->scanline_callback = beamRace;
  g_pDVI->timing=&dvi_timing_340x240p_60hz;
  // For those displays where the upper mode does not work
  //g_pDVI->timing=&dvi_timing_640x480p_60hz;
  g_pDVI->ser_cfg = picodvi_cfg;
  vreg_set_voltage(VREG_VOLTAGE_1_20);                    				
  set_sys_clock_khz(g_pDVI->timing->bit_clk_khz, true); 
  dvi_init(g_pDVI,next_striped_spin_lock_num(), next_striped_spin_lock_num());
  Start();
}

void VideoOut::Start()
{
  // We'll accept the challenge to race the beam for saving RAM
  queue_add_blocking_u32(&g_pDVI->q_colour_valid, &pScanLine); 
  queue_add_blocking_u32(&g_pDVI->q_colour_valid, &pScanLine); 
  multicore_launch_core1(core1_main);
}