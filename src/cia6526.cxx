/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
 * CIA#1 ($DC00) PA0-PA7: JOY0-3, BNT-4 /COL0-COL7
 *               PB0-PB7: ROW0-ROW7
 * 
*/

#include "stdinclude.hxx"

CIA6526::CIA6526(Logging *pLogging, RpPetra *pGlue)
{
    m_pLog=pLogging;
    m_pGlue=pGlue;
}

CIA6526::~CIA6526() {}


void CIA6526::Reset() 
{
  m_i64Clks=0;
}

// Default: IRQ 
void CIA6526::SignalInterrupt(bool signal)
{
  m_pGlue->SignalIRQ(signal);
}

void __not_in_flash_func (CIA6526::handleTimerA)()
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
      // Do we need to trigger timer B?
      if (m_registerSet[0x0f] & 0x20)
      {
          uint16_t timer=m_registerSet[0x07]*256+m_registerSet[0x06];
          if (timer-->0) // timer B will fire anyway in a us...
          {
            m_registerSet[0x06]=timer % 256;
            m_registerSet[0x07]=timer / 256;
          }
      }

      if (m_registerSetWrite[0x0d] & 0x01) // IRQ timer A allowed?
      {
        m_registerSet[0x0d]|=0x81; // Signal IRQ, timer A
        SignalInterrupt(true);
        
        if (m_registerSet[0x0e] & 0x08) // Single shot timer?
        {
            m_registerSet[0x0e]&=0xfe; // Indicate timer stopped
        }
      }
      m_registerSet[0x04]=m_registerSetWrite[0x04];
      m_registerSet[0x05]=m_registerSetWrite[0x05];
    }
  }
}

void __not_in_flash_func (CIA6526::handleTimerB)()
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
        m_registerSet[0x0d]|=0x82; // Signal IRQ, timer B
        SignalInterrupt(true);

        if (m_registerSet[0x0f] & 0x08) // Single shot timer?
        {
            m_registerSet[0x0f]&=0xfe; // Indicate timer stopped
        }
      }
      m_registerSet[0x06]=m_registerSetWrite[0x06];
      m_registerSet[0x07]=m_registerSetWrite[0x07];
    }
  }
}

void __not_in_flash_func (CIA6526::Clk)() 
{
  m_i64Clks++;
  
  if (m_registerSet[0x0e] & 0x01) // Start timer set?
    handleTimerA();

  if (m_registerSet[0x0f] & 0x01) // Start timer set?
    handleTimerB();
}

uint8_t CIA6526::ReadRegister(uint8_t reg) 
{
  uint8_t ret=m_registerSet[reg];
  if (reg==0x0d)
  {
    if (ret & 0x80)
    {
      SignalInterrupt(false);
    }
    m_registerSet[reg]=0x00;
  }
  return ret;
}

void __not_in_flash_func (CIA6526::WriteRegister)(uint8_t reg, uint8_t value)
{
  switch (reg)
  {
    case 0x0d:
      if (value & 0x80) // bit 7=1: each bit 0..4 will set the bits 0-4, 0: deletes the bits 0..4
      {
        // Set bits 0..4
        if (value & 0x01)
        {
          m_registerSetWrite[reg]|=0x01;
        }
        if (value & 0x02)
        {
          m_registerSetWrite[reg]|=0x02;
        }
        if (value & 0x04)
        {
          m_registerSetWrite[reg]|=0x04;
        }
        if (value & 0x08)
        {
          m_registerSetWrite[reg]|=0x08;
        }
        if (value & 0x10)
        {
          m_registerSetWrite[reg]|=0x10;
        }
      }
      else {
        // Clear bits 0..4
        if (value & 0x01)
        {
          m_registerSetWrite[reg]&=0xfe;
        }
        if (value & 0x02)
        {
          m_registerSetWrite[reg]&=0xfd;
        }
        if (value & 0x04)
        {
          m_registerSetWrite[reg]&=0xfb;
        }
        if (value & 0x08)
        {
          m_registerSetWrite[reg]&=0xf7;
        }
        if (value & 0x10)
        {
          m_registerSetWrite[reg]&=0xef;
        }
      }
    break;

    case 0x05: 
    case 0x07:
      // In case high byte timer a/b is set, also high byte of timer is set in case timer is stopped
      m_registerSetWrite[reg]=value; // latch
      if (reg==0x05) // timer A
      {
        if ((m_registerSet[0x0e] &  0x01)==0) // timer A stopped?
        {
          m_registerSet[reg-1]=m_registerSetWrite[reg-1];        
        }
      }
      else // timer B
      {
        if ((m_registerSet[0x0f] &  0x01)==0) // timer b stopped?
        {
          m_registerSet[reg-1]=m_registerSetWrite[reg-1];        
        }
      }
    case 0x04:
    case 0x06:
      m_registerSetWrite[reg]=value;        
    break;

    case 0x0e:
    case 0x0f:
      if (value & 10) // Load latch into timer (strobe)?
      {
        if (reg==0x0e)
        {
          m_registerSet[0x04]=m_registerSetWrite[0x04];
          m_registerSet[0x05]=m_registerSetWrite[0x05];
        }
        else
        {
          m_registerSet[0x06]=m_registerSetWrite[0x06];
          m_registerSet[0x07]=m_registerSetWrite[0x07];
        }
      }
      m_registerSet[reg]=(value & 0xef); // Bit 4 will always read 0
    break;

    default:
      m_registerSet[reg]=value;
    break;
  }
}

