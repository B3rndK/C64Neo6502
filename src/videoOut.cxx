/**
 * Generic video output for 340x240 resolution.
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/
#include "stdinclude.hxx"

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

  // 25 lines
  upperBorderStop=12;
  lowerBorderStart=220;
  
  if ((_pGlue->m_pVICII->m_registerSetRead[0x11] & 0b00001000)==0)  // Display is 24 lines
  {
    upperBorderStop=20;
    lowerBorderStart=212;
  }

  leftBorderEnd=28; // 38 cols
  if (_pGlue->m_pVICII->m_registerSetRead[0x16] & 0b00001000) // 40 cols
  {
    leftBorderEnd=20;
  }

  bool isScreenSwitchedOff=((currentBeamPos<=upperBorderStop) || (currentBeamPos>=lowerBorderStart) || !(_pGlue->m_pVICII->m_registerSetRead[0x11] & 0x10));

  uint16_t color=colorIndex[_pGlue->m_pVICII->m_borderColor[currentBeamPos+39] & 0x0f];
  uint32_t *pP=(uint32_t *)pScanLine;
  uint32_t pixel=(color<<16 | color)<< 31 | ((color << 16 | color));
  uint16_t idx=0;
  // Blit the border(s) or blank the screen, 32-bits at a time (loop unrolled)
  for (int i=0;i<8;i++) 
  {
    pP[idx]=pixel;
    pP[idx+1]=pixel;
    pP[idx+2]=pixel;
    pP[idx+3]=pixel;
    pP[idx+4]=pixel;
    pP[idx+5]=pixel;
    pP[idx+6]=pixel;
    pP[idx+7]=pixel;
    pP[idx+8]=pixel;
    pP[idx+9]=pixel;
    pP[idx+10]=pixel;
    pP[idx+11]=pixel;
    pP[idx+12]=pixel;
    pP[idx+13]=pixel;
    pP[idx+14]=pixel;
    pP[idx+15]=pixel;
    pP[idx+16]=pixel;
    pP[idx+17]=pixel;
    pP[idx+18]=pixel;
    pP[idx+19]=pixel;
    pP[idx+20]=pixel;
    idx+=21;
  }
  // 336
  pP[idx]=pixel;
  pP[idx+1]=pixel;

  while (queue_try_remove_u32(&g_pDVI->q_colour_free, &pScanLine));  
  
  if (!isScreenSwitchedOff)
  {
    if (currentBeamPos<lowerBorderStart-upperBorderStop && currentBeamPos>upperBorderStop)
    {
      uint16_t x=leftBorderEnd/3;
      uint8_t *pCurBuffer = frameBuffer+((currentBeamPos-upperBorderStop)*160);
      uint8_t pixel; 
      // 2 pixels encoded in 4-bits... This way we can easily support even VIC's FLI color modes later on...
      for (int i=0;i<320/2;i+=8) {
          pixel=pCurBuffer[i]; 
          pScanLine[x]=colorIndex[pixel>>4];
          pScanLine[x+1]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+1]; 
          pScanLine[x+2]=colorIndex[pixel>>4];
          pScanLine[x+3]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+2]; 
          pScanLine[x+4]=colorIndex[pixel>>4];
          pScanLine[x+5]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+3]; 
          pScanLine[x+6]=colorIndex[pixel>>4];
          pScanLine[x+7]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+4]; 
          pScanLine[x+8]=colorIndex[pixel>>4];
          pScanLine[x+9]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+5]; 
          pScanLine[x+10]=colorIndex[pixel>>4];
          pScanLine[x+11]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+6]; 
          pScanLine[x+12]=colorIndex[pixel>>4];
          pScanLine[x+13]=colorIndex[pixel & 15];

          pixel=pCurBuffer[i+7]; 
          pScanLine[x+14]=colorIndex[pixel>>4];
          pScanLine[x+15]=colorIndex[pixel & 15];
          x+=16;

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