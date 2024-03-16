#ifndef _COMPUTER_HXX
#define _COMPUTER_HXX

class RpPetra;
extern RpPetra *_pGlue;

class Computer {
  
  public: 
    Logging *m_pLogging;    
  
  public:
    Computer(Logging *pLogging);
    virtual ~Computer();
    
    int Run();

    // We need a CPU first...
    RP65C02 *m_pCPU;

    // We need a custom chip for RAM/ROM interleave
    // Petra *m_pPetra;
    // We need a custom chip for graphics
    // Sabrina *m_pSabrina;

    // Petra, our custom chip keeping it all together
    RpPetra *m_pGlue;

    int m_WaitCycles;

    uint64_t inline GetTotalCycles() { return m_totalCyles;}

  private:
    
    uint64_t m_totalCyles;    
    SYSTEMSTATE m_systemState;
    repeating_timer_t m_CpuClkTimerInfo;
    
    // Methods go here
    int Init();
    void CreateCPU();
    void Execute();
   
};

typedef Computer * PCOMPUTER;

#endif
