 /**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/
#include "stdinclude.hxx"
#include "roms.hxx"
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

#define SOUND_PIN 20

// static uint8_t benchmark[]={0xA9, 0x00, 0xAA, 0xA8, 0xE8, 0xD0, 0xFD,0xC8,0xD0,0xFA,0xAA,0xE8,0x8A,0xC9,0xFF,0xD0,0xF3,0x8D,0x20,0xD0,0x4C,0x04,0xE0};

static char Charset[]={ '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                        '[','#',']','#','#',' ','!','"','#','$','%','&','`','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',
                        ':',';','<','=','>','?'};

RpPetra::RpPetra(Logging *pLogging, RP65C02 *pCPU)
{
    m_pLog=pLogging;
    m_pCPU=pCPU;
    m_pCIA1 = new CIA1(pLogging,this);
    m_pCIA2 = new CIA2(pLogging,this);
    m_pVICII= new VIC6569(pLogging,this);
    m_pKeyboard= new Keyboard(pLogging, this);
    m_pColorRam= (uint8_t *)calloc(1000,sizeof(uint8_t));
    m_pJoystickA=nullptr;
    m_pJoystickB=nullptr;
    m_pRAM=(uint8_t *)calloc(65536,sizeof(uint8_t));
    m_pRAM[1]=0x37;

#ifdef _MONITOR_CARTRIDGE
    // SYS 49152
    memcpy(&m_pRAM[0xc000],mon_c000,sizeof(mon_c000));
#endif


#ifdef _SIMONS_BASIC
  memcpy(&m_pRAM[0x8000],simons_basic,sizeof(simons_basic));
  // Prevent autostart by changing 'CBM80' => 'CBM81'.
  // Needs to be repoked back to 48 by a poke 32776,48 and then sys64738 to start the module
  // After Simon's basic is started, enter "old" to recover a little basic program.
  m_pRAM[0x8008]++;
  memcpy(&m_pRAM[0x0801],apfel_0801,sizeof(apfel_0801));
  memcpy(&m_pRAM[0xC000],basic_old,sizeof(basic_old));
#else

#ifdef _RASTERIRQ 
uint8_t rasterIrq[]={ 0x78,0xA9,0x22,0x8D,0x14,0x03,0xA9,0x40,
                      0x8D,0x15,0x03,0xA9,0x00,0x8D,0x12,0xD0,
                      0xAD,0x11,0xD0,0x29,0x7F,0x8D,0x11,0xD0,
                      0xAD,0x1A,0xD0,0x09,0x01,0x8D,0x1A,0xD0,
                      0x58,0x60,0xAD,0x19,0xD0,0x30,0x03,0x4C,
                      0x63,0x40,0x8D,0x19,0xD0,0xAD,0x12,0xD0,
                      0xD0,0x10,0xA9,0x00,0x8D,0x20,0xD0,0x8D,
                      0x21,0xD0,0xA9,0x70,0x8D,0x12,0xD0,0x4C,
                      0x63,0x40,0xC9,0x70,0xD0,0x10,0xA9,0x02,
                      0x8D,0x20,0xD0,0x8D,0x21,0xD0,0xA9,0xD0,
                      0x8D,0x12,0xD0,0x4C,0x63,0x40,0xA9,0x07,
                      0x8D,0x20,0xD0,0x8D,0x21,0xD0,0xA9,0x00,
                      0x8D,0x12,0xD0,0x68,0xA8,0x68,0xAA,0x68,
                      0x40};

  memcpy(&m_pRAM[0x4000],rasterIrq,sizeof(rasterIrq));
#endif

#ifdef _ELITE
  memcpy(&m_pRAM[0x4000],elite_pic_4000,sizeof(elite_pic_4000));
  memcpy(&m_pRAM[0xc000],picldr_c000,sizeof(picldr_c000));
#endif

#ifdef _FLASHDANCE
  memcpy(&m_pRAM[0x0801],flashdance_rom,sizeof(flashdance_rom));
  memcpy(&m_pRAM[0xC000],flashdance_old,sizeof(flashdance_old));
#endif

#ifdef _SYNTH_SAMPLE
  memcpy(&m_pRAM[0x0801],synth_rom,sizeof(synth_rom));
  memcpy(&m_pRAM[0xC000],synth_old,sizeof(synth_old));
#endif

#ifdef _TRAPDOOR
  memcpy(&m_pRAM[0x0801],trapdoor,sizeof(trapdoor));
#endif


#endif
  m_pVideoOut=new VideoOut(pLogging, this, m_pVICII->GetFrameBuffer());
  Reset();
}

/*
  The following two procedures are taken from https://github.com/paulscottrobson/neo6502-firmware
  and were written by Paul Robson (paul@robsons.org.uk) and Harry Fairhead.
*/

void pwm_interrupt_handler() {
  static uint16_t buffer;
  pwm_clear_irq(pwm_gpio_to_slice_num(SOUND_PIN));
  SIDCalcBuffer((uint8_t*)&buffer, 2); 
  pwm_set_gpio_level(SOUND_PIN, buffer);
}

void SNDInitialise(void) {

#ifdef _NO_SID 
  return;
#else

  SIDInit();

  gpio_set_function(SOUND_PIN, GPIO_FUNC_PWM);
  int audio_pin_slice = pwm_gpio_to_slice_num(SOUND_PIN);

  // Setup PWM interrupt to fire when PWM cycle is complete
  pwm_clear_irq(audio_pin_slice);
  pwm_set_irq_enabled(audio_pin_slice, true);
  // set the handle function above
  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
  irq_set_enabled(PWM_IRQ_WRAP, true);

  // Setup PWM for audio output
  pwm_config config = pwm_get_default_config();
  /* Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
    * to set the interrupt rate. 
    * 
    * 11 KHz is fine for speech. Phone lines generally sample at 8 KHz
    * 
    * 
    * So clkdiv should be as follows for given sample rate
    *  8.0f for 11 KHz
    *  4.0f for 22 KHz
    *  2.0f for 44 KHz etc
    */
        
    pwm_init(audio_pin_slice, &config, true);
    pwm_config_set_clkdiv(&config, 23.0f);
    pwm_config_set_wrap(&config, 250); 
    pwm_set_gpio_level(SOUND_PIN, 0);
#endif    
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
  //SIDReset(0);
  ::SNDInitialise();

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

/** Part of the PLA... 
 * 
 * Bit0: LoRam - 1: A000-BFFF ROM, 0: RAM
 * Bit1: HiRam - 1: E000-FFFF ROM, 0: RAM
 * Bit2: CharEn- 1: D000-DFFF CHAREN, 0: RAM/IO
 * 
*/

bool RpPetra::IsIOVisible()
{
  if (((m_cpuAddr & 1)==0) && ((m_cpuAddr & 2)==0))
  {
    return false;
  }
  return ((m_cpuAddr & 0x04));
}

bool RpPetra::IsBasicRomVisible()
{
  return ((m_cpuAddr & 0x01) && (m_cpuAddr & 0x02));
}

bool RpPetra::IsKernalRomVisible()
{
  return  (m_cpuAddr & 0x02);
}

bool RpPetra::IsCharRomVisible()
{
  bool ret=false;
  switch (m_cpuAddr)
  {
    case 1:
    case 2:
    case 3:
      ret=true;
    break;
  }
  return (ret);
}

#ifdef _NEVER
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
#endif 

// In this design we use Petra's CLK == 65C02 PHI2. We may later decide
// to use some kind of interleave factor x.
void RpPetra::Clk(bool isRisingEdge, SYSTEMSTATE *pSystemState, uint64_t totalCycles)
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
  if (!handled)
  {
    if (IsIOVisible())
    {
      if (addr>=0xd000 && addr<=0xd02e) // VICII 6569
      {
        handled=true;
        if (pSystemState->cpuState.readNotWrite) {   // READ access
          //sprintf (szBuffer,"VIC-II read access: %04x\n",addr);
          WriteDataBus(m_pVICII->m_registerSetRead[addr-0xd000]);
        }
        else {
          m_pVICII->WriteRegister(addr-0xd000,pSystemState->cpuState.d0d7);
        }
      }
      else if (addr>=0xd400 && addr<=0xd41c) // SID6581/6582/8580
      {
        handled=true;
#ifndef _NO_SID      
        if (pSystemState->cpuState.readNotWrite) {   // READ access
          sid_read((uint32_t)addr-0xd400, (cycle_t)totalCycles);
        }
        else {
          sid_write((uint32_t)addr-0xd400,pSystemState->cpuState.d0d7);
        }
#endif
      } 
      else if (addr>=0xd800 && addr<=0xdbe7) // Colorram
      {
        handled=true;
        if (pSystemState->cpuState.readNotWrite) {   // READ access in colorram
          WriteDataBus(m_pColorRam[addr-0xd800]);
        }
        else // write to colorram
        {
          WriteDataBus(m_pRAM[addr]);
          m_pColorRam[addr-0xd800]=pSystemState->cpuState.d0d7;
        }
      }
      else if (addr>=0xdc00 && addr<=0xdcff) // CIA-1
      {   
        handled=true;
        if (pSystemState->cpuState.readNotWrite) {   // READ access
          // CIA addresses are mirrored every 16 byte until dcff.
          WriteDataBus(m_pCIA1->ReadRegister((addr-0xdc00) % 16));
        }
        else {
          m_pCIA1->WriteRegister((addr-0xdc00) % 16, pSystemState->cpuState.d0d7);
        }
      }
      else if (addr>=0xdd00 && addr<=0xddff) // CIA-2
      {
        handled=true;
        if (pSystemState->cpuState.readNotWrite) {   // READ access
          WriteDataBus(m_pCIA2->ReadRegister((addr-0xdd00) % 16));
        }
        else {
          m_pCIA2->WriteRegister((addr-0xdd00) % 16, pSystemState->cpuState.d0d7);
        }
      }
    }
    if (!handled)
    {
      if (addr>=0xd000 && addr<=0xdfff && IsCharRomVisible())
      {
        handled=true;
        if (pSystemState->cpuState.readNotWrite) {   // READ access
          WriteDataBus(chargen_rom[addr-0xd000]);
        }
        else
        {
          WriteDataBus(m_pRAM[addr]);
        }
      }
    }
    if (!handled) // No IO access
    {
      if (addr>=0xa000 && addr<=0xbfff)  // maybe basic ROM access or ram
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
      else if (addr>=0xd000 && addr<=0xdfff)
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
      else if (addr>=0xe000 && addr<=0xffff)    
      {
        if (addr>=0xfffe) // IRQ vector
        {
          if (pSystemState->cpuState.readNotWrite)  // READ IRQ vector
          { 
            if (IsKernalRomVisible())                  
            {
              WriteDataBus(kernal_rom[addr-0xe000]);
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
        else if (addr==0xfffa || addr==0xfffb)  // NMI vector  
        {
#ifdef _NMISTART
          if (!HandleModuleStart(addr, pSystemState->cpuState.readNotWrite))
#endif
          {

            if (pSystemState->cpuState.readNotWrite)  // READ NMI vector
            {
              if (IsKernalRomVisible())                  
              {
                WriteDataBus(kernal_rom[addr-0xe000]);
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
        }
        else
        {
          if (pSystemState->cpuState.readNotWrite)  
          {
            if (IsKernalRomVisible())                  
            {
              WriteDataBus(kernal_rom[addr-0xe000]);
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
      }
      else // everything else
      {
          exit(-1);
      }
    }
  }
}

// Called in case of an NMI (F7) for module starts
bool RpPetra::HandleModuleStart(uint16_t addr, bool isRead)
{
  bool ret=false;
#ifdef _NMISTART
  if (isRead)
  {
    static int nmiStage=0;  
    uint8_t nmiStageMax=2;
#ifdef _ELITE
    nmiStageMax=4;
#endif
    if (nmiStage<=nmiStageMax)
    {
#ifdef _ELITE
      uint8_t magic[]={0x68,0x68,0x68,0xa9,0xC0,0x48,0xa9,0x00,0x48,0x08,0x40};  // $C000
      uint8_t magic_run[]={0x68,0x68,0x68,0xa9,0x01,0x48,0xa9,0xb6,0x48,0x08,0x40};  // $01B6
#elif _TRAPDOOR    
      // 24688, $6070  
      uint8_t magic[]={0x68,0x68,0x68,0xa9,0x60,0x48,0xa9,0x70,0x48,0x08,0x40}; 
#elif _FLASHDANCE 
      uint8_t magic[]={0x78,0x48,0xa9,0xbc,0x8d,0x14,0x03,0xa9,0x75,0x8d,0x15,0x03,0xa9,0x0f,0x8d,0x18,0xd4,0x68,0x58,0x40}; 
#elif _WIZBALL
      // 25488 $6390
      uint8_t magic[]={0x68,0x68,0x68,0xa9,0x63,0x48,0xa9,0x90,0x48,0x08,0x40}; 
#else
      uint8_t magic[]={0x40}; 
#endif
      if (nmiStage==0)
      {
        
#ifdef _FLASHDANCE 
        memcpy(&m_pRAM[0x033c],magic,sizeof(magic));
#endif
#ifndef _RASTERIRQ
        memcpy(&m_pRAM[0x00ff],magic,sizeof(magic));
#endif

#ifdef _ELITE
        memcpy(&m_pRAM[0x4000],elite_pic_4000,sizeof(elite_pic_4000));
        memcpy(&m_pRAM[0xc000],picldr_c000,sizeof(picldr_c000));
        memcpy(m_pColorRam,elite_d800,sizeof(elite_d800));
#endif      

#ifdef _WIZBALL
        memcpy(&m_pRAM[0x0801],wizball_rom,sizeof(wizball_rom));
#endif
      }
#ifdef _ELITE      
      else if (nmiStage==2)
      {
          memcpy(&m_pRAM[0x02],elite,sizeof(elite));
          memcpy(&m_pRAM[0x00ff],magic_run,sizeof(magic_run));
      }
#endif
      nmiStage++;

#ifdef _FLASHDANCE
      uint8_t low=0x3c;
      uint8_t high=0x03;
#else              
      uint8_t low=0xff;
      uint8_t high=0x00;
#endif
      if (addr==0xfffa)
      {
        WriteDataBus(low);    
        ret=true;
      }
      else
      {
        WriteDataBus(high);    
        ret=true;
      }
    }
  }
#endif          
 return ret;
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