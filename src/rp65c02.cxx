#include <stdio.h>
#include <hardware/gpio.h>
#include <pico/time.h>
#include "logging.hxx"
#include "rp65c02.hxx"

RP65C02::RP65C02(Logging *pLogging)
{
  m_pLogging = pLogging;
}

RP65C02::~RP65C02()
{
}
