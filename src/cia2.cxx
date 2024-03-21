#include "stdinclude.hxx"

CIA2::CIA2(Logging *pLogging, RpPetra *pGlue) : CIA6526(pLogging, pGlue)
{
}

CIA2::~CIA2()
{
  m_registerSet[0x00]=0x03; // VIC base address to bank 0
}

// CIA-2 is connected to NMI, not IRQ pin
void CIA2::SignalInterrupt(bool signal)
{
  if (signal)
  {
    m_pGlue->SignalNMI(false);
    m_pGlue->SignalNMI(true);
  }
  else
  {
    m_pGlue->SignalNMI(true);
  }
}

uint8_t CIA2::ReadRegister(uint8_t reg) 
{
  return CIA6526::ReadRegister(reg);
}

void CIA2::handleTimerA()
{
  if (m_registerSetWrite[0x0e] & 0x01) // Start timer set?
  {
    // Interrupt Timer A allowed, decrease counter
    uint16_t timer=m_registerSet[0x05]*256+m_registerSet[0x04];
    if (timer>0 && (m_registerSet[0x0e] & 0x01)) // 0x0e bit 0 indicates start countdown
    {
      timer--;
      m_registerSet[0x04]=timer % 256;
      m_registerSet[0x05]=timer / 256;
      if (timer==0)
      {
        if (m_registerSetWrite[0x0d] & 0x01) // IRQ timer A allowed?
        {
          m_registerSet[0x0d]|=0x01; // IRQ, timer A
          SignalInterrupt(true);
        
          if (m_registerSetWrite[0x0e] & 0x08) // Single shot timer?
          {
              m_registerSetWrite[0x0e]&=0xfe; // Stop timer
          }
          else {
            m_registerSet[0x04]=m_registerSetWrite[0x04];
            m_registerSet[0x05]=m_registerSetWrite[0x05];
          }
        }
      }
    }
  }
}

void CIA2::handleTimerB()
{
  if (m_registerSetWrite[0x0f] & 0x01) // Start timer set?
  {
    // Interrupt Timer B allowed, decrease counter
    uint16_t timer=m_registerSet[0x07]*256+m_registerSet[0x06];
    if (timer>0 && (m_registerSet[0x0e] & 0x01)) // 0x0e bit 0 indicates start countdown
    {
      timer--;
      m_registerSet[0x06]=timer % 256;
      m_registerSet[0x07]=timer / 256;
      if (timer==0)
      {
        if (m_registerSetWrite[0x0d] & 0x02) // IRQ timer B allowed?
        {
          m_registerSet[0x0d]|=0x02; // IRQ, timer B
          SignalInterrupt(true);
        
          if (m_registerSetWrite[0x0f] & 0x08) // Single shot timer?
          {
              m_registerSetWrite[0x0f]&=0xfe; // Stop timer
          }
          else
          {
            m_registerSet[0x06]=m_registerSetWrite[0x06];
            m_registerSet[0x07]=m_registerSetWrite[0x07];
          }
        }
      }
    }
  }
}
