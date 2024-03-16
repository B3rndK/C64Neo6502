#ifndef CIA1_HXX_
#define CIA1_HXX_

class RpPetra;

class CIA1 : public CIA6526 {
  
  public:
    CIA1(Logging *pLogging, RpPetra *pGlue);
    virtual ~CIA1();
    void Reset();
    void Clk();
    uint8_t ReadRegister(uint8_t reg);
};

#endif



