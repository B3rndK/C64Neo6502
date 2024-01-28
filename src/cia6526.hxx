
#ifndef CIA6526_HXX_
#define CIA6526_HXX_

class RpPetra;

class CIA6526 {
  
  private:  
    Logging *m_pLog;
    u_int64_t m_i64Clks;
    u_int8_t m_registerSet[0x10];    
    u_int8_t m_registerSetWrite[0x10];
    RpPetra *m_pGlue;
  
  public:
    CIA6526(Logging *pLogging, RpPetra *pGlue);
    virtual ~CIA6526();
    void Reset();
    void Clk();
    void WriteRegister(u_int8_t reg, u_int8_t value);
    u_int8_t ReadRegister(u_int8_t reg);
};

#endif