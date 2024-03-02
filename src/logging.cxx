/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <initializer_list>
#include "terminalBase.hxx"
#include "ansiTerminal.hxx"
#include "logging.hxx"  

#ifdef _DEBUG

Logging::Logging(TerminalBase *pTerminal, LogLevel logLevel=All)
{
  m_pTerminal=pTerminal;
  m_logLevel=logLevel;
}

Logging::~Logging()
{
  // delete m_pTerminal;
}

void Logging::LogTitle(std::initializer_list<const char *> logOutput)
{
  if (m_logLevel <= All)
  {
    std::cout << m_pTerminal->Bold() << m_pTerminal->Italic();
    Log(logOutput);
  }
}

void Logging::LogDebug(std::initializer_list<const char *> logOutput)
{
  if (m_logLevel <= Debug)
  {
    std::cout << m_pTerminal->Normal();
    Log(logOutput);
  }
}

void Logging::LogError(std::initializer_list<const char *> logOutput)
{
  if (m_logLevel <= Error)
  {
    std::cout << m_pTerminal->Blink() << m_pTerminal->Bold() << m_pTerminal->Red();
    Log(logOutput);
  }
}

void Logging::LogWarning(std::initializer_list<const char *> logOutput)
{
  if (m_logLevel <= Warning)
  {
    std::cout << m_pTerminal->Bold() << m_pTerminal->Yellow();
    Log(logOutput);
  }
}

void Logging::LogInfo(std::initializer_list<const char *> logOutput)
{
  if (m_logLevel <= Info)
  {
    std::cout << m_pTerminal->Italic() << m_pTerminal->Green();
    Log(logOutput);
  }
}

void Logging::FindDaBug(std::initializer_list<const char *> logOutput)
{
  std::cout << m_pTerminal->Bold() << m_pTerminal->White();
  Log(logOutput);
}

void Logging::Log(std::initializer_list<const char *> logOutput)
{
  for (auto out : logOutput)
  {
    std::cout << out;
  }
  std::cout << m_pTerminal->Normal();
  std::flush(std::cout);
}

void Logging::SetLoggingLevel(LogLevel level)
{
  m_logLevel=level;
}

void Logging::Clear()
{
   if (m_logLevel<None) {
      std::cout << m_pTerminal->ClearScreen() << m_pTerminal->CursorHome() << m_pTerminal->Normal();
      std::flush(std::cout); 
   }
}

#else

Logging::Logging(TerminalBase *pTerminal, LogLevel logLevel=All)
{
  m_pTerminal=pTerminal;
}

Logging::~Logging()
{
   // delete m_pTerminal;
}
void Logging::LogDebug(std::initializer_list<const char *> logOutput){};
void Logging::LogInfo(std::initializer_list<const char *> logOutput){};
void Logging::LogError(std::initializer_list<const char *> logOutput){};
void Logging::LogWarning(std::initializer_list<const char *> logOutput){};
void Logging::LogTitle(std::initializer_list<const char *> logOutput){};
void Logging::FindDaBug(std::initializer_list<const char *> logOutput){};
void Logging::Clear(){};

#endif
