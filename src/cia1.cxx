#include "stdinclude.hxx"

#define ALL_READ 0
#define ALL_WRITE 0xff

CIA1::CIA1(Logging *pLogging, RpPetra *pGlue) : CIA6526(pLogging,pGlue)
{
  m_registerSet[0x00]=0xff; // Pull-up setting all high.
  m_registerSet[0x01]=0xff; // Pull-up setting all high.
}

CIA1::~CIA1()
{

}

uint8_t __not_in_flash_func (CIA1::ReadRegister)(uint8_t reg) 
{
  uint8_t ret = m_registerSet[reg];
  
  if (reg==0x01)  // Data PORT B
  {
    if (m_registerSet[2]>0)  
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
            uint8_t mask=~m_registerSet[0];
            uint8_t row=~keys->row;
            if (row & mask)
            {
              ret&=keys->col;
            }
          }
        }
      }
    }
  }
  else if (reg==0x00)
  {
    if (m_pGlue->m_pJoystickA!=nullptr)
    {
      JoystickStatus status=m_pGlue->m_pJoystickA->m_status;
      ret=status.up+status.down*2+status.left*4+status.right*8+status.fire*16;
    }
    else // No joystick attached.
    {
      ret=0x7f;
    }
    m_registerSet[0]=ret;
  }
  else
  {
    ret=CIA6526::ReadRegister(reg);
  }
  return ret;
}
