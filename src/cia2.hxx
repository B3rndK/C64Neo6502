#ifndef CIA2_HXX_
#define CIA2_HXX_

class RpPetra;

class CIA2 : public CIA6526 {
  
  private:  
    void SignalInterrupt(bool signal);
  public:
    CIA2(Logging *pLogging, RpPetra *pGlue);
    virtual ~CIA2();
};

#endif