#ifndef _TERMINAL_BASE_HXX
#define _TERMINAL_BASE_HXX

// Fully virtual base class for different terminal types.

class TerminalBase {

  public:
    
    // Colors
    virtual const char *White()=0;
    virtual const char *Black()=0;
    virtual const char *Blue()=0;
    virtual const char *Red()=0;
    virtual const char *Green()=0;
    virtual const char *Yellow()=0;
    virtual const char *Default()=0;

    // Attributes
    virtual const char *Reverse()=0;
    virtual const char *Normal()=0;
    virtual const char *Blink()=0;
    virtual const char *Bold()=0;
    virtual const char *Italic()=0;

    // Clear
    virtual const char *ClearScreen()=0;
    virtual const char *CursorHome()=0;
    virtual const char *Reset()=0;
};

#endif