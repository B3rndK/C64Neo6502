 /**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

#include <hardware/gpio.h>
#include <pico/time.h>
#include <dvi.h>
#include <dvi_serialiser.h>
#include <memory.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "videoOut.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"
#include "monitor.hxx"
#include "roms/basic.hxx"
#include "roms/kernal.hxx"
#include "roms/simons_basic.hxx"
#include "roms/apfel_0801.hxx"

extern uint8_t chargen_rom[];
extern uint8_t simons_basic[];
/* 1:19 at 1,06Mhz , 0:41,51 at 2,04 Mhz (252000kHz), 1:27,85 at 0,985 Mhz, C64 Original: 1:30,51

  .org 0xe000
  lda #$00
  tax
  tay
  loop1:
  inx
  bne loop1
  iny
  bne loop1
  tax
  inx
  txa
  cmp #$FF
  bne loop1
  sta $d020
  jmp loop1

*/

// static uint8_t benchmark[]={0xA9, 0x00, 0xAA, 0xA8, 0xE8, 0xD0, 0xFD,0xC8,0xD0,0xFA,0xAA,0xE8,0x8A,0xC9,0xFF,0xD0,0xF3,0x8D,0x20,0xD0,0x4C,0x04,0xE0};

static char Charset[]={ '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                        '[','#',']','#','#',' ','!','"','#','$','%','&','`','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',
                        ':',';','<','=','>','?'};

RpPetra::RpPetra(Logging *pLogging, RP65C02 *pCPU)
{
    m_pLog=pLogging;
    m_pCPU=pCPU;
    m_pCIA1 = new CIA6526(pLogging, this);
    m_pCIA2 = new CIA6526(pLogging, this);
    m_pVICII= new VIC6569(pLogging, this);
    m_pRAM=(uint8_t *)calloc(65536,sizeof(uint8_t));
    m_pRAM[1]=55;

#ifdef _MONITOR_CARTRIDGE
    memcpy(&m_pRAM[0xc000],mon_c000,sizeof(mon_c000));
#endif

#ifdef _SIMONS_BASIC
    memcpy(&m_pRAM[0x8000],simons_basic,sizeof(simons_basic));
    // Prevent autostart by changing 'CBM80' => 'CBM81'.
    // Needs to be repoked back to 48 by a poke 32776,48 and then sys64738 to start the module
    // After Simon's basic is started, enter "old" to recover a little basic program.
    m_pRAM[0x8008]++;
    memcpy(&m_pRAM[0x0801],apfel_0801,sizeof(apfel_0801));
#endif 

    m_pVideoOut=new VideoOut(pLogging, this, m_pVICII->GetFrameBuffer());
    Reset();
}

RpPetra::~RpPetra()
{
}

void RpPetra::Reset()
{
  // Activate RP2040 pins used for the CPU communication
  gpio_init_mask(pioMask_CPU);   
  gpio_set_dir(CLK,true); // CLK is always an output signal
  gpio_set_dir(RESET,true); // RESET is always an output signal
  gpio_set_dir(IRQ,true); // IRQ is always an output signal
  gpio_set_dir(NMI,true); // NMI is always an output signal
  gpio_set_dir(RW,false);  // RW pin is always input from 6502
  gpio_set_dir_out_masked(pioMaskOE_U5_U6_U7);  // Set OE pins to output 
  gpio_set_dir_in_masked(pioMaskData_U5_U6_U7); // Switch all pio pins from transceiver data lines to input
  SignalIRQ(false);
  SignalNMI(false);
  DisableBus();
  m_pVICII->Reset();  
  m_pCIA1->Reset();  
  m_pCIA2->Reset();  
  m_pVideoOut->Reset();
  ResetCPU();
}


// Note: According to the WDC the IRQB low level should be held until the interrupt handler clears 
// the interrupt request source.
void RpPetra::SignalIRQ(bool enable)
{
  if (enable) gpio_put(IRQ, LOW); // IRQ is low active so reset is active now
  else gpio_put(IRQ, HIGH); // IRQ is low active so reset is active now
}

// WDC: A negative transition on the Non-Maskable Interrupt (NMIB) input initiates an interrupt sequence after the
// current instruction is completed
void RpPetra::SignalNMI(bool enable)
{
  if (enable) gpio_put(NMI, LOW); // IRQ is low active so reset is active now
  else gpio_put(NMI, HIGH); // IRQ is low active so reset is active now
}

// Reset logic for 65C02 
void RpPetra::ResetCPU()
{
  // According to the WDC databook, we need to keep RESB low for at least two
  // cycles. However, for the 65C02 silicon used in the neo6502 this is not necessary. 
  // A little delay would do without requiring any PHI2 (CLK).
  gpio_put(RESET, LOW); // RESET is low active so reset is active now
  ClockCPU(2);
  gpio_put(RESET, HIGH);
  // We now have to run six (in fact 7) clock cycles before any data from CPU is valid. Source: WDC 65C02 databook.
  ClockCPU(7);
}

// Triggers the PHI2 the number of times specified gracefully. It looks like the
// 65C02 requires some time during RESET not relying to CLK.
void RpPetra::ClockCPU(int counter)
{
  for (int i=0;i<counter;i++)
  {
    PHI2(LOW);
    sleep_ms(10);
    PHI2(HIGH);
    sleep_ms(10);
  }
}

/** Part of the PLA... */
bool RpPetra::IsIOVisible()
{
  static bool table[8]{false,false,false,false,false,true,true,true};
  return(table[m_cpuAddr]);
}

/** Part of the PLA... */
bool RpPetra::IsBasicRomVisible()
{
  static bool table[8]{false,false,false,true,false,false,false,true};
  return(table[m_cpuAddr]);
}

/** Part of the PLA... */
bool RpPetra::IsKernalRomVisible()
{
  static bool table[8]{false,false,true,true,false,false,true,true};
  return(table[m_cpuAddr]);
}

bool RpPetra::IsCharRomVisible()
{
  static bool table[8]{false,true,true,true,false,false,false,false};
  return(table[m_cpuAddr]);
}


// In this design we use Petra's CLK == 65C02 PHI2. We may later decide
// to use some kind of interleave factor x.
void RpPetra::Clk(bool isRisingEdge, SYSTEMSTATE *pSystemState)
{
  int_fast16_t addr;
  //static char szBuffer[256];
  // szBuffer[0]=0;
  PHI2(LOW);
  m_pCIA1->Clk();
  m_pCIA2->Clk();
  m_pVICII->Clk();
  pSystemState->cpuState.isRisingEdge=isRisingEdge;
  gpio_set_dir_in_masked(pioMaskData_U5_U6_U7); // set the datalines to input (seen from RP2040 side)   
  PHI2(HIGH);
  ReadCPUSignals(pSystemState);      
  addr=pSystemState->cpuState.a0a15;
  m_cpuAddr=m_pRAM[1] & 0x07;
  bool handled=false;

  if ((addr>=0x0000 && addr<=0x9fff) || (addr>=0xc000 && addr<=0xcfff))
  {
      handled=true;
      if (pSystemState->cpuState.readNotWrite) {   // READ access
         WriteDataBus(m_pRAM[addr]);
      }
      else {
        m_pRAM[addr]=pSystemState->cpuState.d0d7;
      }
  }
  else if (IsIOVisible())
  {
    if (addr>=0xd000 && addr<=0xd02e && IsIOVisible()) // VIC II
    {
      handled=true;
      if (pSystemState->cpuState.readNotWrite) {   // READ access
        //sprintf (szBuffer,"VIC-II read access: %04x\n",addr);
        WriteDataBus(m_pVICII->m_registerSetRead[addr-0xd000]);
      }
      else {
        //sprintf (szBuffer,"VIC-II write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
        m_pVICII->WriteRegister(addr-0xd000,pSystemState->cpuState.d0d7);
        m_screenUpdated=true;
      }
      //m_pLog->LogInfo({szBuffer});
    }
    else if (addr>=0xdc00 && addr<=0xdcff && IsIOVisible()) // CIA-1
    {   
      handled=true;
      if (pSystemState->cpuState.readNotWrite) {   // READ access
        //sprintf (szBuffer,"CIA1 read access: %04x\n",addr);
        // CIA addresses are mirrored every 16 byte until dcff.
        WriteDataBus(m_pCIA1->ReadRegister((addr-0xdc00) % 16));
      }
      else {
        //sprintf (szBuffer,"CIA1 write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
        m_pCIA1->WriteRegister((addr-0xdc00) % 16, pSystemState->cpuState.d0d7);
      }
      //m_pLog->LogInfo({szBuffer});
    }
    else if (addr>=0xdd00 && addr<=0xddff && IsIOVisible()) // CIA-2
    {
      handled=true;
      if (pSystemState->cpuState.readNotWrite) {   // READ access
          // CIA addresses are mirrored every 16 byte until ddff.
        WriteDataBus(m_pCIA2->ReadRegister((addr-0xdd00) % 16));
        //sprintf (szBuffer,"CIA2 read access: %04x\n",addr);
      }
      else {
        //sprintf (szBuffer,"CIA2 write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
        m_pCIA2->WriteRegister((addr-0xdd00) % 16, pSystemState->cpuState.d0d7);
      }
      //m_pLog->LogInfo({szBuffer});
    }
  }
  if (!handled)
  {
    if (addr>=0xd000 && addr<=0xdfff)
    {
        if (pSystemState->cpuState.readNotWrite)  // READ 
        {
          if (IsCharRomVisible())                  
          {
            // Character ROM
            WriteDataBus(chargen_rom[addr-0xd000]);
          }
          else
          {
            WriteDataBus(m_pRAM[addr]);
          }
        }
        else
        {
          // Non IO access, so always write to RAM
          m_pRAM[addr]=pSystemState->cpuState.d0d7;
        }
    }
    else if (addr>=0xa000 && addr<=0xbfff)  // maybe basic ROM access or ram
    {
        if (pSystemState->cpuState.readNotWrite)  // READ 
        {
          if (IsBasicRomVisible())                  
          {
            // Character ROM
            WriteDataBus(basic_rom[addr-0xa000]);
          }
          else
          {
            WriteDataBus(m_pRAM[addr]);
          }
        }
        else
        {
          m_pRAM[addr]=pSystemState->cpuState.d0d7;
        }
    }
    else if (addr>=0xe000 && addr<=0xffff)    
    {
        if (pSystemState->cpuState.readNotWrite)  // READ 
        {
          if (IsKernalRomVisible())                  
          {
            // Character ROM
            WriteDataBus(kernal_rom[addr-0xe000]);
          }
          else
          {
            WriteDataBus(m_pRAM[addr]);
          }
        }
        else
        {
          // Always write to RAM 
          m_pRAM[addr]=pSystemState->cpuState.d0d7;
        }
    }
    else  // std. RAM
    {
        if (pSystemState->cpuState.readNotWrite)  // READ 
        {
          WriteDataBus(m_pRAM[addr]);
        }
        else
        {
          m_pRAM[addr]=pSystemState->cpuState.d0d7;
        }
    }
  }
}

void RpPetra::ReadCPUSignals(SYSTEMSTATE *pSystemState)
{
   // read A0-7
  Enable_U5_only();
  DELAY_FACTOR_TRANSCEIVER()
  pSystemState->cpuState.a0a15 = (gpio_get_all() & pioMaskData_U5_U6_U7_RW);
  pSystemState->cpuState.readNotWrite=pSystemState->cpuState.a0a15 & 0x800;
  // read A8-15
  Enable_U6_only();
  DELAY_FACTOR_TRANSCEIVER();
  pSystemState->cpuState.a0a15 &= 0xf7ff;
  pSystemState->cpuState.a0a15 |= (gpio_get_all() & pioMaskData_U5_U6_U7) << 8;
  // In case 65C02 indicates a write, read databus as well.
  if (!pSystemState->cpuState.readNotWrite)
  {
      Enable_U7_only();
      DELAY_FACTOR_TRANSCEIVER();
      pSystemState->cpuState.d0d7=(gpio_get_all() & pioMaskData_U5_U6_U7);    
  } 
}

void RpPetra::WriteDataBus(uint8_t byte)
{
  gpio_set_dir_out_masked(pioMaskData_U5_U6_U7); // set the datalines to output (seen from RP2040 side)
  gpio_put_masked(pioMaskData_U5_U6_U7,(uint32_t)byte);
  Enable_U7_only();
}


void RpPetra::UpdateScreen()
{
  if (m_screenUpdated)
  {
    m_screenUpdated=false;
    m_pLog->Clear();
    DumpScreen();
  }
}

void RpPetra::DumpScreen()
{
  char row[41];
  row[40]=0;
  
  for (int y=0; y<25; y++)
  {
    for (int x=0; x<40; x++)
    {
      row[x]=Charset[m_pRAM[0x400+((40*y)+x)]];      
    }
    m_pLog->LogInfo({row});
    m_pLog->LogInfo({"\n\r"});
  }
}