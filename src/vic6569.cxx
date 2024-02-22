#include <stdio.h>
#include <string.h>
#include <hardware/gpio.h>
#include <dvi.h>
#include <dvi_serialiser.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "videoOut.hxx"
#include "rpPetra.hxx"
#include "roms/chargen.hxx"

VIC6569::VIC6569(Logging *pLogging, RpPetra *pGlue)
{
   m_pLog=pLogging;
   m_pGlue=pGlue;
   // We need a frame buffer of only 160*200 due to 4-bit per Pixel
   m_pFrameBuffer=(u_int8_t *)calloc((160*200),sizeof(u_int8_t));
}

VIC6569::~VIC6569() {};

void VIC6569::Reset() 
{
  m_i64Clks=0;
  memset(m_registerSetWrite,0,sizeof(m_registerSetWrite));
  memset(m_registerSetRead,0,sizeof(m_registerSetRead));
  // set current scan line to 0
  m_currentScanLine=0;
  m_pGlue->m_screenUpdated=true;
}

void VIC6569::UpdateFrameBuffer()
{
  uint8_t value=m_registerSetRead[0x11];
  static int16_t linesUpdated=-1;
  
  if (m_pGlue->m_screenUpdated)
  {
    m_pGlue->m_screenUpdated=false;
    linesUpdated=0;
  }
  
  if (linesUpdated>=0) {
    if (value & 0x10) // At least, display is on
    {
      if (m_currentScanLine>END_SCANLINE_UPPER_BORDER_PAL && m_currentScanLine<START_SCANLINE_LOWER_BORDER_PAL)
      {
        if ((value & 0b01100000)==0) // Textmode?
        {
          HandleTextMode(value & 0b01000000);
        }
        else 
        {
          HandleHiresMode(value & 0b01000000);
        }
      }
    }
    if (linesUpdated++==NUM_OF_VLINES_PAL)
    {
      linesUpdated=-1;
    }
  }
}  

void VIC6569::HandleTextMode(bool multicolor)
{
    u_int8_t *pCurrentLine=m_pFrameBuffer+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))*160);
    uint16_t curRow=(m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))/8;
    u_int16_t scanbufferOffset=0;

    for (int i=0;i<40;i++) 
    {
      int offset=curRow*40+i;
      
      u_int8_t characterInVideoRam=m_pGlue->m_pRAM[0x400+offset];
      u_int8_t bits=chargen_rom[(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)];
      
      u_int8_t foregroundColor=m_pGlue->m_pRAM[0xd800+offset] & 0x0f;
      u_int8_t backgroundColor=m_registerSetRead[0x21] & 0x0f;
    
      
      if (bits & 0x80)
      {
        pCurrentLine[scanbufferOffset]=foregroundColor << 4;
      }
      else
      {
        pCurrentLine[scanbufferOffset]=backgroundColor << 4;
      }        
      if (bits & 0x40)
      {
        pCurrentLine[scanbufferOffset]|=foregroundColor;
      }
      else
      {
        pCurrentLine[scanbufferOffset]|=backgroundColor;
      }        
      
      scanbufferOffset++;

      if (bits & 0x20)
      {
        pCurrentLine[scanbufferOffset]=foregroundColor << 4;
      }
      else
      {
        pCurrentLine[scanbufferOffset]=backgroundColor << 4;
      }        
      if (bits & 0x10)
      {
        pCurrentLine[scanbufferOffset]|=foregroundColor;
      }
      else
      {
        pCurrentLine[scanbufferOffset]|=backgroundColor;
      }        
      scanbufferOffset++;

      if (bits & 0x8)
      {
        pCurrentLine[scanbufferOffset]=foregroundColor << 4;
      }
      else
      {
        pCurrentLine[scanbufferOffset]=backgroundColor << 4;
      }        
      if (bits & 0x4)
      {
        pCurrentLine[scanbufferOffset]|=foregroundColor;
      }
      else
      {
        pCurrentLine[scanbufferOffset]|=backgroundColor;
      }        
      scanbufferOffset++;
      if (bits & 0x2)
      {
        pCurrentLine[scanbufferOffset]=foregroundColor << 4;
      }
      else
      {
        pCurrentLine[scanbufferOffset]=backgroundColor << 4;
      }        
      if (bits & 0x1)
      {
        pCurrentLine[scanbufferOffset]|=foregroundColor;
      }
      else
      {
        pCurrentLine[scanbufferOffset]|=backgroundColor;
      }        
      scanbufferOffset++;
  }
}



void VIC6569::HandleHiresMode(bool multicolor)
{
    // Not yet.
}

void VIC6569::Clk() 
{
  m_i64Clks++;
  
  // Every 63 clocks the VIC starts a new line
  
  if (m_i64Clks % CLOCKS_PER_HLINE==0)
  {   
    m_currentScanLine++;

    if (m_currentScanLine>NUM_OF_VLINES_PAL)
    {
      m_currentScanLine=0;
      m_registerSetRead[0x11]&=0x7F;
      m_registerSetRead[0x12]=0;
    }
    else if (m_currentScanLine>0xFF)
    {
      m_registerSetRead[0x11]|=0x80;
      m_registerSetRead[0x12]|=m_currentScanLine % 0x100;
    }
    else 
    {
      m_registerSetRead[0x12]=m_currentScanLine;
    }
    // Now check if we need to signal an IRQ due to vertical line count
    if (m_registerSetRead[0x1a] & 0x01)
    {
      int irqAtScanline=m_registerSetWrite[0x12];
      if (m_registerSetWrite[0x11] & 0x80)
      {
        irqAtScanline+=256;
      }
      if (irqAtScanline==m_currentScanLine)
      {
        // Indicate raster match
        m_registerSetWrite[0x19]|=0x01;
        // Now check if we need to signal an IRQ due to vertical line count
        if (m_registerSetWrite[0x1a] & 0x01) 
        {
          m_pGlue->SignalIRQ(true); 
        }
      }
    }
    UpdateFrameBuffer();
  }
}

// reg 0x16 Bit 3, 40 (1) or 38 columns (0)
void VIC6569::WriteRegister(u_int8_t reg, u_int8_t value)
{
  switch (reg)
  {
    case 0x19:
      m_registerSetWrite[reg]=value;
      if (value & 0x01)
      {
        m_pGlue->SignalIRQ(false); 
      }
    break;

    case 0x11:
    case 0x12:
      m_registerSetWrite[reg]=value;
      m_registerSetRead[reg]=value;  
    break;

    default:
      m_registerSetRead[reg]=value;  
  }
 
}
