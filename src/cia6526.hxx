
#ifndef CIA6526_HXX_
#define CIA6526_HXX_

class RpPetra;

class CIA6526 {
  
  private:  
    Logging *m_pLog;
    uint64_t m_i64Clks;
    uint8_t m_registerSet[0x10];    
    uint8_t m_registerSetWrite[0x10];
    RpPetra *m_pGlue;
  
  public:
    CIA6526(Logging *pLogging, RpPetra *pGlue);
    virtual ~CIA6526();
    void Reset();
    void Clk();
    void WriteRegister(uint8_t reg, uint8_t value);
    uint8_t ReadRegister(uint8_t reg);
};

#endif