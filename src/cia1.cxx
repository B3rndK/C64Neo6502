#include "stdinclude.hxx"

CIA1::CIA1(Logging *pLogging, RpPetra *pGlue) : CIA6526(pLogging,pGlue)
{
  m_registerSet[0x00]=0xff; // Pull-up setting all high.
  m_registerSet[0x01]=0xff; // Pull-up setting all high.
}

CIA1::~CIA1()
{

}

uint8_t CIA1::ReadRegister(uint8_t reg) 
{
  uint8_t ret = m_registerSet[reg];
  
  if (reg==0x01 && m_registerSet[2]==0xff) //  // 0=column (port A, $02 ctrl), 1=row (port B, $03 ctrl)
  {
      std::vector<Keys *> keysPressed=m_pGlue->m_pKeyboard->GetKeysPressed();
      ret=0xff;
      if (!keysPressed.empty())
      {        
          if (m_registerSet[0]==0x00) // C64 is probing if any key has been pressed...
          {
            ret=0x20;
          }
          else
          {
            for(auto keys : keysPressed)
            {
              if (keys->row==m_registerSet[0])
              {
                ret&=keys->col;
              }
            }
          }
      }
      m_registerSet[1]=ret;
  }
  else if (reg==0x00)
  {
    JoystickStatus status=m_pGlue->m_pJoystickA->m_status;
    ret=status.up+status.down*2+status.left*4+status.right*8+status.fire*16;
    m_registerSet[0]=ret;
  }
  
  if (reg==0x0d)
  {
    if (ret & 0x80)
    {
      m_pGlue->SignalIRQ(false);
    }
    m_registerSet[reg]=0x00;
  }

  return ret;
}
