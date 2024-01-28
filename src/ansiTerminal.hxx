#ifndef _ANSI_TERMINAL_HXX
#define _ANSI_TERMINAL_HXX

#include "terminalBase.hxx"

// Implementation of an ansi terminal base implementation 

class AnsiTerminal: public TerminalBase 
{
  public:
    
    AnsiTerminal(){};
    
    // Colors   

    const char *White() {
      return "\033[37m";
    };

    const char *Black() {
      return "\033[30m";
    };

    const char *Blue() {
      return "\033[34m";
    };

    const char *Red() {
      return "\033[31m";
    };

    const char *Green() {
      return "\033[32m";
    };
    
    const char *Yellow() {
      return "\033[33m";
    };

    const  char *Default() {
      return "\033[39m";
    };

    // Attributes
    
    const char *Reverse() {
      return "\033[7m";
    };

    const char *Normal() {
      return "\033[0m";
    };

    const char *Blink() {
      return "\033[5m";
    };

    const char *Bold() {
      return "\033[1m";
    };

    const char *Italic() {
      return "\033[3m";
    };

    // Clear

    const char *ClearScreen() {
      return "\033[2J";
    };

    const char *CursorHome() {
      return "\033[H";
    };

    const char *Reset() {
      return "\033[0m";
    };


};

#endif