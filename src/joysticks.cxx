/**
 * Written by Bernd Krekeler, Herne, Germany.
*/
#include "stdinclude.hxx"


Joysticks::Joysticks(Logging *pLogging, uint8_t port)
{
  m_pLog=pLogging;
  m_port=port;
}

Joysticks::~Joysticks()
{

}

void Joysticks::ClearButtonStatus(void)
{
  m_status={PORT_2,NOT_PRESSED,NOT_PRESSED,NOT_PRESSED,NOT_PRESSED,NOT_PRESSED};  
}



