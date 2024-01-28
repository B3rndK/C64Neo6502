#include <hardware/gpio.h>
#include <pico/time.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"
#include "monitor.hxx"
#include "roms/chargen.hxx"
#include "roms/basic.hxx"
#include "roms/kernal.hxx"

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

static u_int8_t benchmark[]={0xA9, 0x00, 0xAA, 0xA8, 0xE8, 0xD0, 0xFD,0xC8,0xD0,0xFA,0xAA,0xE8,0x8A,0xC9,0xFF,0xD0,0xF3,0x8D,0x20,0xD0,0x4C,0x04,0xE0};

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

    m_pRAM=(u_int8_t *)calloc(1,65536);
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

// In this design we use Petra's CLK == 65C02 PHI2. We may later decide
// to use some kind of interleave factor x.
void RpPetra::Clk(bool isRisingEdge, SYSTEMSTATE *pSystemState)
{
  int_fast16_t addr;
  static int_fast16_t lastaddr;
  static bool lastWasWrite=false;
  char szBuffer[256];
  static int64_t start=0;
  PHI2(LOW);
  m_pCIA1->Clk();
  m_pCIA2->Clk();
  m_pVICII->Clk();
  pSystemState->cpuState.isRisingEdge=isRisingEdge;
  gpio_set_dir_in_masked(pioMaskData_U5_U6_U7); // set the datalines to input (seen from RP2040 side)   
  PHI2(HIGH);
  ReadCPUSignals(pSystemState);      
  addr=pSystemState->cpuState.a0a15;

  if (addr>=0xd000 && addr<=0xd02e) // VIC II
  {
    if (pSystemState->cpuState.readNotWrite) {   // READ access
      sprintf (szBuffer,"VIC-II read access: %04x\n",addr);
      WriteDataBus(m_pVICII->ReadRegister(addr-0xd000));
    }
    else {
      sprintf (szBuffer,"VIC-II write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
      m_pVICII->WriteRegister(addr-0xd000,pSystemState->cpuState.d0d7);
    }
    //m_pLog->LogInfo({szBuffer});
  }
  else if (addr>=0xdc00 && addr<=0xdcff) // CIA-1
  {   
    if (pSystemState->cpuState.readNotWrite) {   // READ access
      sprintf (szBuffer,"CIA1 read access: %04x\n",addr);
      // CIA addresses are mirrored every 16 byte until dcff.
      WriteDataBus(m_pCIA1->ReadRegister((addr-0xdc00) % 16));
    }
    else {
      sprintf (szBuffer,"CIA1 write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
      m_pCIA1->WriteRegister((addr-0xdc00) % 16, pSystemState->cpuState.d0d7);
    }
    //m_pLog->LogInfo({szBuffer});
  }
  else if (addr>=0xdd00 && addr<=0xddff) // CIA-2
  {
    if (pSystemState->cpuState.readNotWrite) {   // READ access
        // CIA addresses are mirrored every 16 byte until ddff.
      WriteDataBus(m_pCIA2->ReadRegister((addr-0xdd00) % 16));
      sprintf (szBuffer,"CIA2 read access: %04x\n",addr);
    }
    else {
      sprintf (szBuffer,"CIA2 write access: %04x,%02x\n",addr,pSystemState->cpuState.d0d7);
      m_pCIA2->WriteRegister((addr-0xdd00) % 16, pSystemState->cpuState.d0d7);
    }
    //m_pLog->LogInfo({szBuffer});
  }
  else {
    // access to standard RAM or ROM

    if (pSystemState->cpuState.readNotWrite) 
    {  
      // READ access
      if (addr>=0xe000)
      {
        // Kernal ROM
        WriteDataBus(kernal_rom[addr-0xe000]);
      }
      else if (addr>=0xa000 && addr<=0xbfff)
      {
        // Basic ROM 
        WriteDataBus(basic_rom[addr-0xa000]);
      }
      else 
      {
        // Regular DRAM
        if (pSystemState->cpuState.readNotWrite) {  
          WriteDataBus(m_pRAM[addr]);
        }
      }
    }
    else // Write to memory address
    {
      if (addr>=0xa000 && addr<=0xbfff)
      {
      }
      else if (addr>=0xe000 && addr<=0xffff)
      {
      }
      else
      { // Write to RAM
        m_pRAM[addr]=pSystemState->cpuState.d0d7;
      }
    }
    
    if (lastWasWrite && (lastaddr>=0x0400 && lastaddr<=0x07AC) && (addr<0x0400 || addr>0x07AC)) // Last access to video ram
    {
        m_screenUpdated=true;
    }
    lastaddr=addr;
    lastWasWrite=!pSystemState->cpuState.readNotWrite;
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