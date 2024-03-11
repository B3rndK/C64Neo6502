/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

#include "stdinclude.hxx"

CompetitionPro::CompetitionPro(Logging *pLog) : Joysticks(pLog)
{
}

void CompetitionPro::Convert(uint8_t const * report, uint16_t len)
{
  ClearButtonStatus();
  m_status.port=PORT_2; // testing only

  if (report[0x05]!=0x0f)
  {
    m_status.fire=PRESSED;
  }
  if (report[0x03]==0x00)
  {
    m_status.left=PRESSED;
  }
  else if (report[0x03]==0xff)
  {
    m_status.right=PRESSED;
  }
  if (report[0x04]==0xff)
  {
    m_status.down=PRESSED;
  }
  else if (report[0x04]==0x00)
  {
    m_status.up=PRESSED;
  }
}
