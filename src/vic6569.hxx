
#ifndef VIC6569_HXX_
#define VIC6569_HXX_

#define CLOCKS_PER_HLINE 63
#define NUM_OF_VLINES_PAL 312

class RpPetra;

class VIC6569 {
  
  private:  
    Logging *m_pLog;
    RpPetra *m_pGlue; 
    u_int64_t m_i64Clks;
    u_int8_t m_registerSet[0x2f];
    u_int8_t m_registerSetWrite[0x2f];
    u_int16_t currentScanLine;

  public:
    VIC6569(Logging *pLogging, RpPetra *pGlue);
    virtual ~VIC6569();
    void Reset();
    void Clk();
    void WriteRegister(u_int8_t reg, u_int8_t value);
    inline u_int8_t ReadRegister(u_int8_t reg) {return m_registerSet[reg];};

};

#endif