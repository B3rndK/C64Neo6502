#ifndef LOGGING_HXX_
#define LOGGING_HXX_

#include "terminalBase.hxx"
#include <iostream>
#include <initializer_list>

typedef enum  {
  All,
  Debug,
  Info,
  Warning,
  Error,
  None
} LogLevel;

class Logging {

  public:

    Logging(TerminalBase *pTerminal, LogLevel logLevel);
    virtual ~Logging();
  
    /// @brief Dynamically change the logging level
    /// @param level Logging level to use
    void SetLoggingLevel(LogLevel level);

    /// @brief Sends output to stdio
    /// @param logOutput List of what to log
    void LogDebug(std::initializer_list<const char *> logOutput);
    void LogError(std::initializer_list<const char *> logOutput);
    void LogWarning(std::initializer_list<const char *> logOutput);
    void LogInfo(std::initializer_list<const char *> logOutput);
    void LogTitle(std::initializer_list<const char *> logOutput);
    
    /// @brief Will always log, independent of the logging level
    /// @param logOutput 
    void FindDaBug(std::initializer_list<const char *> logOutput);
    
    /// @brief Write CR and some LFs to "clear" the terminal
    void Clear();

  private:
    TerminalBase *m_pTerminal;
    LogLevel m_logLevel;
    void Log(std::initializer_list<const char *> logOutput);


};

#endif

