#include <stdio.h>
#include <hardware/gpio.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "rpPetra.hxx"
#include <string.h>


VIC6569::VIC6569(Logging *pLogging, RpPetra *pGlue)
{
   m_pLog=pLogging;
   m_pGlue=pGlue;
}

VIC6569::~VIC6569() {};

void VIC6569::Reset() 
{
  m_i64Clks=0;

  memset(m_registerSet,0,sizeof(m_registerSet));
  memset(m_registerSetWrite,0,sizeof(m_registerSet));
  // set current scan line to 0
  currentScanLine=0;
}

void VIC6569::Clk() 
{
  m_i64Clks++;
  
  // Every 63 clocks the VIC starts a new line
  
  if (m_i64Clks % CLOCKS_PER_HLINE==0)
  {
    currentScanLine++;

    if (currentScanLine>NUM_OF_VLINES_PAL)
    {
      m_pGlue->UpdateScreen();
      currentScanLine=0;
      m_registerSet[0x11]&=0x7F;
      m_registerSet[0x12]=0;
    }
    else if (currentScanLine>0xFF)
    {
      m_registerSet[0x11]|=0x80;
      m_registerSet[0x12]=currentScanLine % 0x100;
    }
    else 
    {
      m_registerSet[0x12]=currentScanLine;
    }
    // Now check if we need to signal an IRQ due to vertical line count
    if (m_registerSet[0x1a] & 0x01)
    {
      int irqAtScanline=m_registerSetWrite[0x12];
      if (m_registerSetWrite[0x11] & 0x80)
      {
        irqAtScanline+=256;
      }
      if (irqAtScanline==currentScanLine)
      {
        // Indicate raster match
        m_registerSet[0x19]|=0x01;
        // Now check if we need to signal an IRQ due to vertical line count
        if (m_registerSet[0x1a] & 0x01) 
        {
          m_pGlue->SignalIRQ(true); 
        }
      }
    }
  }
}

void VIC6569::WriteRegister(u_int8_t reg, u_int8_t value)
{
  if (reg==0x1a)
  {
    // m_pLog->LogInfo({"VIC interrupts register"});
  }
  if (reg==0x19)
  {
    m_registerSetWrite[reg]=value;
    if (value & 0x01)
    {
      m_pGlue->SignalIRQ(false); 
    }
  }
  if (reg==0x12 || reg==0x11)
  {
    m_registerSetWrite[reg]=value;
  }
  else m_registerSet[reg]=value;
}
