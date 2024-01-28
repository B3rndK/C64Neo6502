#include <hardware/gpio.h>
#include <hardware/adc.h>
#include <pico/stdlib.h>
#include "logging.hxx"
#include "rp65c02.hxx"
#include "cia6526.hxx"
#include "vic6569.hxx"
#include "rpPetra.hxx"
#include "computer.hxx"

#define CPU_CLK_IN_HZ 20000

Computer::Computer(Logging *pLogging)
{
    static u_int8_t loop[]={0xAD,0x00,0xE0,0xAA,0x8E,0x22,0xD0,0xAD,0x21,0xD0,0xA8,0x8C,0x23,0xD0,0x4C,0x00,0xE0};
    m_pLogging=pLogging;
    m_pRAM=&loop[0];
    adc_init();    
    m_systemState.cpuState.isA0A15SetToOutput=true;
}

Computer::~Computer()
{
}

int Computer::Run()
{
  Init();

  do {
    m_pGlue->Clk(HIGH,&m_systemState);
    m_pGlue->Clk(LOW,&m_systemState);
  } while (1);

  return (0);
}

int Computer::Init()
{
  // Create the CPU
  m_pCPU= new RP65C02(m_pLogging);
  // Create the Petra custom chip (glue logic)
  m_pGlue= new RpPetra(m_pLogging, m_pCPU);
  return 0;
}