/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

#include "stdinclude.hxx"

VIC6569::VIC6569(Logging *pLogging, RpPetra *pGlue)
{
   m_pLog=pLogging;
   m_pGlue=pGlue;
   // We need a frame buffer of only 160*200 due to 4-bit per Pixel
   m_pFrameBuffer=(uint8_t *)calloc((160*200),sizeof(uint8_t));
}

VIC6569::~VIC6569() {};

void VIC6569::Reset() 
{
  m_i64Clks=0;
  memset(m_registerSetWrite,0,sizeof(m_registerSetWrite));
  memset(m_registerSetRead,0,sizeof(m_registerSetRead));
  // set current scan line to 0
  m_currentScanLine=0;
}

void VIC6569::UpdateFrameBuffer()
{
  uint8_t value=m_registerSetRead[0x11];
  
  if (value & 0x10) // At least, display is on
  {
    if (m_currentScanLine>END_SCANLINE_UPPER_BORDER_PAL && m_currentScanLine<START_SCANLINE_LOWER_BORDER_PAL)
    {
      if ((value & 0b00100000)==0) // Textmode?
      {
        // Extended color mode?
        if (value & 0b01000000)
        {
          HandleExtendedColorMode(); // extended color mode (ECM)?
        }
        else {
          HandleTextMode(m_registerSetRead[0x16] & 0b00010000); // multicolor text mode?
        }
      }
      else 
      {
        HandleHiresModes(m_registerSetRead[0x16] & 0b00010000);
      }
    }
  }
}  

/**
 * ECM with only 64 characters
 * 
*/
void VIC6569::HandleExtendedColorMode()
{
    uint8_t backgroundColors[4];
    
    uint8_t *pCurrentLine=m_pFrameBuffer+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))*160);
    uint16_t curRow=(m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))/8;
    uint16_t scanbufferOffset=0;
    
    uint16_t characterRamOffset=GetTextModeCharRamAddrOffset();
    uint16_t vicBaseAddress=GetVideoRamStartAddr(false); // VIC bank physical address
    uint8_t bits;
    uint16_t videoRam=GetVideoRamAddrOffset()+vicBaseAddress; // 0x400...

    backgroundColors[0]=m_registerSetRead[0x21] & 0x0f;
    backgroundColors[1]=m_registerSetRead[0x22] & 0x0f;
    backgroundColors[2]=m_registerSetRead[0x23] & 0x0f;
    backgroundColors[3]=m_registerSetRead[0x24] & 0x0f;
    uint8_t backgroundColor;

    for (int i=0;i<40;i++) 
    {
      int offset=curRow*40+i;
      
      uint8_t characterInVideoRam=m_pGlue->m_pRAM[videoRam+offset];
      uint8_t foregroundColor=m_pGlue->m_pColorRam[offset] & 0x0f;
      backgroundColor=backgroundColors[characterInVideoRam/64];
      characterInVideoRam%=64;
     
      if ((characterRamOffset==0x1000 || characterRamOffset==0x1800) && (vicBaseAddress==0x0000 || vicBaseAddress==0x8000))
      {
        bits=chargen_rom[(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)+characterRamOffset-0x1000];
      }
      else
      {
        bits=m_pGlue->m_pRAM[vicBaseAddress+characterRamOffset+(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)];
      }

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


void VIC6569::HandleMulticolorTextMode()
{
    uint8_t *pCurrentLine=m_pFrameBuffer+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))*160);
    uint16_t curRow=(m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))/8;
    uint16_t scanbufferOffset=0;
    uint8_t pixel;

    uint16_t characterRamOffset=GetTextModeCharRamAddrOffset();
    uint16_t vicBaseAddress=GetVideoRamStartAddr(false); // VIC bank physical address
    uint8_t bits;
    uint16_t videoRam=GetVideoRamAddrOffset()+vicBaseAddress; // 0x400...

    uint8_t backgroundColor=m_registerSetRead[0x21] & 0x0f;
    uint8_t color1=m_registerSetRead[0x22] & 0x0f; // 01
    uint8_t color2=m_registerSetRead[0x23] & 0x0f; // 10
    
    for (int i=0;i<40;i++) 
    {
      int offset=curRow*40+i;
      
      uint8_t characterInVideoRam=m_pGlue->m_pRAM[videoRam+offset];

      if ((characterRamOffset==0x1000 || characterRamOffset==0x1800) && (vicBaseAddress==0x0000 || vicBaseAddress==0x8000))
      {
        bits=chargen_rom[(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)+characterRamOffset-0x1000];
      }
      else
      {
        bits=m_pGlue->m_pRAM[vicBaseAddress+characterRamOffset+(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)];
      }
      
      uint8_t color3=m_pGlue->m_pColorRam[curRow*40+i] % 0b00001111; // 11

      if (color3 & 8)
      {
          // this character has to be drawn in multicolor mode.
          // We have 8 bits forming 4 pixel
          for (int j=0;j<4;j++)
          {
            pixel=bits & 0xc0;
            switch (pixel)
            {
              case 0x00:
                pCurrentLine[scanbufferOffset]=backgroundColor << 4;
                pCurrentLine[scanbufferOffset]|=backgroundColor;
              break;
              case 0x40:
                pCurrentLine[scanbufferOffset]=color1 << 4;
                pCurrentLine[scanbufferOffset]|=color1;
              break;
              case 0x80:
                pCurrentLine[scanbufferOffset]=color2 << 4;
                pCurrentLine[scanbufferOffset]|=color2;
              break;
              case 0xc0:
                pCurrentLine[scanbufferOffset]=color3 << 4;
                pCurrentLine[scanbufferOffset]|=color3;
              break;
            }
            scanbufferOffset++;
            bits<<=2;
          }
      }
      else  // standard text
      {
        if (bits & 0x80)
        {
          pCurrentLine[scanbufferOffset]=color3 << 4;
        }
        else
        {
          pCurrentLine[scanbufferOffset]=backgroundColor << 4;
        }        
        if (bits & 0x40)
        {
          pCurrentLine[scanbufferOffset]|=color3;
        }
        else
        {
          pCurrentLine[scanbufferOffset]|=backgroundColor;
        }        
        
        scanbufferOffset++;

        if (bits & 0x20)
        {
          pCurrentLine[scanbufferOffset]=color3 << 4;
        }
        else
        {
          pCurrentLine[scanbufferOffset]=backgroundColor << 4;
        }        
        if (bits & 0x10)
        {
          pCurrentLine[scanbufferOffset]|=color3;
        }
        else
        {
          pCurrentLine[scanbufferOffset]|=backgroundColor;
        }        
        scanbufferOffset++;

        if (bits & 0x8)
        {
          pCurrentLine[scanbufferOffset]=color3 << 4;
        }
        else
        {
          pCurrentLine[scanbufferOffset]=backgroundColor << 4;
        }        
        if (bits & 0x4)
        {
          pCurrentLine[scanbufferOffset]|=color3;
        }
        else
        {
          pCurrentLine[scanbufferOffset]|=backgroundColor;
        }        
        scanbufferOffset++;
        if (bits & 0x2)
        {
          pCurrentLine[scanbufferOffset]=color3 << 4;
        }
        else
        {
          pCurrentLine[scanbufferOffset]=backgroundColor << 4;
        }        
        if (bits & 0x1)
        {
          pCurrentLine[scanbufferOffset]|=color3;
        }
        else
        {
          pCurrentLine[scanbufferOffset]|=backgroundColor;
        }        
        scanbufferOffset++;
      }
  } 
}

void VIC6569::HandleStandardTextMode()
{
    uint8_t *pCurrentLine=m_pFrameBuffer+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))*160);
    uint16_t curRow=(m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1))/8;
    uint16_t scanbufferOffset=0;
    uint16_t characterRamOffset=GetTextModeCharRamAddrOffset();
    uint16_t vicBaseAddress=GetVideoRamStartAddr(false); // VIC bank physical address
    uint8_t bits;
    uint16_t videoRam=GetVideoRamAddrOffset()+vicBaseAddress; // 0x400...
    
    for (int i=0;i<40;i++) 
    {
      int offset=curRow*40+i;
      
      uint8_t characterInVideoRam=m_pGlue->m_pRAM[videoRam+offset];
      
      if ((characterRamOffset==0x1000 || characterRamOffset==0x1800) && (vicBaseAddress==0x0000 || vicBaseAddress==0x8000))
      {
        bits=chargen_rom[(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)+characterRamOffset-0x1000];
      }
      else
      {
        bits=m_pGlue->m_pRAM[vicBaseAddress+characterRamOffset+(8*characterInVideoRam)+((m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1)) % 8)];
      }

      uint8_t foregroundColor=m_pGlue->m_pColorRam[offset] & 0x0f;
      uint8_t backgroundColor=m_registerSetRead[0x21] & 0x0f;
      
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

void VIC6569::HandleTextMode(bool multicolor)
{
    if (multicolor)
    {
        HandleMulticolorTextMode();
    }    
    else
    {
        HandleStandardTextMode();
    }
}


/* CIA-B $DD00 bits 0..1 define the four possible 16k banks */
uint16_t VIC6569::GetVideoRamStartAddr(bool bitmapMode)
{
  static uint16_t addrTab[4]={0xc000,0x8000,0x4000,0x0000};
  uint16_t addr=addrTab[m_pGlue->m_pCIA2->ReadRegister(0) & 0b00000011];
  if (bitmapMode)
  {
    if (m_registerSetRead[0x18] & 0b00001000)
    {
      addr+=0x2000;
    }
  }
  
  return (addr);
}

uint16_t videoRamStartAddr=0;


/** Where to find the charset character definition.
 *  $d018- bits 1-3 (text mode)
 */
uint16_t VIC6569::GetTextModeCharRamAddrOffset()
{
  static uint16_t table[8]={0x0000,0x0800,0x1000,0x1800,0x2000,0x2800,0x3000,0x3800};
  return table[(m_registerSetRead[0x18]>>1) & 0x07];
}

/**
 *  Video RAM (Text) bits 4..7 of 0xd018
*/
uint16_t VIC6569::GetVideoRamAddrOffset()
{
  static uint16_t table[16]={0x0000,0x0400,0x0800,0x0c00,0x1000,0x1400,0x1800,0x1c00,0x2000,0x2400,0x2800,0x2c00,0x3000,0x3400,0x3800,0x3c00};
  return table[(m_registerSetRead[0x18]>>4) & 0b00001111];
}

void VIC6569::HandleHiresModes(bool multicolor)
{
  if (multicolor)
  {
    HandleMulticolorBitmapMode();
  } 
  else
  {
    HandleStandardBitmapMode();
  }    
}

/**
 * Standard Bitmap mode is 320x200/16
*/
void VIC6569::HandleStandardBitmapMode()
{
    uint8_t scanLine=m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1);
    uint8_t *pCurrentLine=m_pFrameBuffer+((scanLine)*160);
    videoRamStartAddr=GetVideoRamStartAddr(true);
    uint16_t vicBaseAddr=GetVideoRamStartAddr(false);
    uint16_t curRow=(scanLine/8); // 0-24
    uint16_t scanbufferOffset=0;
    uint16_t curVidMem=videoRamStartAddr+(curRow*320);
    curVidMem+=(scanLine % 8);
    uint8_t bits;
    uint16_t videoRamAddrOffset=GetVideoRamAddrOffset(); // 0x400,0x800...

    for (int i=0;i<40;i++) 
    {
      if (vicBaseAddr==0x0000 || vicBaseAddr==0x8000) // Bank 0,2
      {
        if (curVidMem>=vicBaseAddr+0x1000 && curVidMem<=vicBaseAddr+0x2000)
        {
          bits=chargen_rom[(curVidMem-0x1000)];
        }
        else
        {
          bits=m_pGlue->m_pRAM[curVidMem];  
        }
      }
      else
      {
        bits=m_pGlue->m_pRAM[curVidMem];
      }

      uint8_t foregroundColor=(m_pGlue->m_pRAM[vicBaseAddr+videoRamAddrOffset+(curRow*40+i)] & 0b11110000) >> 4;
      uint8_t backgroundColor=m_pGlue->m_pRAM[vicBaseAddr+videoRamAddrOffset+(curRow*40+i)] & 0b00001111;
      
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
      curVidMem+=8;
    }
}


/**
*   Multicolor Bitmap Mode is 160x200/16
*/
void VIC6569::HandleMulticolorBitmapMode()
{
    uint8_t scanLine=m_currentScanLine-(END_SCANLINE_UPPER_BORDER_PAL+1);
    uint8_t *pCurrentLine=m_pFrameBuffer+((scanLine)*160);
    videoRamStartAddr=GetVideoRamStartAddr(true);
    uint16_t curRow=(scanLine/8); // 0-24
    uint16_t scanbufferOffset=0;
    uint16_t curVidMem=videoRamStartAddr+(curRow*320);
    uint16_t vicBaseAddr=GetVideoRamStartAddr(false);
    uint16_t videoRamAddrOffset=GetVideoRamAddrOffset(); // 0x400,0x800...

    curVidMem+=(scanLine % 8);
    uint8_t bits;

    uint8_t backgroundColor=m_registerSetRead[0x21] & 0b00001111; // 00
    uint8_t pixel;

    for (int i=0;i<40;i++) 
    {
      uint8_t color1=(m_pGlue->m_pRAM[vicBaseAddr+videoRamAddrOffset+(curRow*40+i)] & 0b11110000) >> 4; // 01
      uint8_t color2=m_pGlue->m_pRAM[vicBaseAddr+videoRamAddrOffset+(curRow*40+i)] & 0b00001111; // 10
      uint8_t color3=m_pGlue->m_pColorRam[curRow*40+i] & 0b00001111; // 11


      if (vicBaseAddr==0x0000 || vicBaseAddr==0x8000) // Bank 0,2
      {
        if (curVidMem>=vicBaseAddr+0x1000 && curVidMem<=vicBaseAddr+0x2000)
        {
          bits=chargen_rom[(curVidMem-0x1000)];
        }
        else
        {
          bits=m_pGlue->m_pRAM[curVidMem];  
        }
      }
      else
      {
        bits=m_pGlue->m_pRAM[curVidMem];
      }

      for (int j=0;j<4;j++)
      {
        pixel=bits & 0xc0;
        switch (pixel)
        {
          case 0x00:
            pCurrentLine[scanbufferOffset]=backgroundColor << 4;
            pCurrentLine[scanbufferOffset]|=backgroundColor;
          break;
          case 0x40:
            pCurrentLine[scanbufferOffset]=color1 << 4;
            pCurrentLine[scanbufferOffset]|=color1;
          break;
          case 0x80:
            pCurrentLine[scanbufferOffset]=color2 << 4;
            pCurrentLine[scanbufferOffset]|=color2;
          break;
          case 0xc0:
            pCurrentLine[scanbufferOffset]=color3 << 4;
            pCurrentLine[scanbufferOffset]|=color3;
          break;
        }
        scanbufferOffset++;
        bits<<=2;
      }
      curVidMem+=8;      
    }
}


void VIC6569::Clk() 
{
  bool signalIRQ=false; 
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
      m_registerSetRead[0x11]&=0b01111111;
      m_registerSetRead[0x12]=m_currentScanLine;
    }
    // Now check if we need to signal an IRQ due to vertical line count
    if (m_registerSetWrite[0x1a] & 0x01)
    {
      int irqAtScanline=m_registerSetWrite[0x12]; // registerSetWrite: When shall the next IRQ occur?
      if (m_registerSetWrite[0x11] & 0x80)
      {
        irqAtScanline+=256;
      }
      signalIRQ=irqAtScanline==m_currentScanLine;
    }
    if (signalIRQ)
    {
      // Indicate raster match to irq routine
      m_registerSetRead[0x19]|=0x81;
      m_pGlue->SignalIRQ(true);
      
    }
  }
  if (m_i64Clks % CLOCKS_PER_HLINE==62 || m_i64Clks==0) {
    m_borderColor[m_currentScanLine]=m_registerSetRead[0x20];
    UpdateFrameBuffer(); // Update every scanline
  }
}

// reg 0x16 Bit 3, 40 (1) or 38 columns (0)
void VIC6569::WriteRegister(uint8_t reg, uint8_t value)
{
  switch (reg)
  {
    case 0x19:
      m_registerSetWrite[reg]=~value;
      m_registerSetRead[reg]=m_registerSetWrite[reg];
      if (!(value & 0x01)) 
      {
        m_pGlue->SignalIRQ(false);   
      }
    break;

    case 0x1a:
      m_registerSetWrite[reg]=value;
    break;

    case 0x11:
      m_registerSetWrite[reg]=value;
      m_registerSetRead[reg]=value;  
    break;
    case 0x12:
      m_registerSetWrite[reg]=value;
    break;
    default:
      m_registerSetRead[reg]=value;  
  }
 
}
