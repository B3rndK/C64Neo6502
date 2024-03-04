/**
 * PETRA custom chip, serving as the glue logic.
 * It contains:
 *  - RESET logic for 6502
*/

#ifndef _PETRA_HXX
#define _PETRA_HXX

typedef struct 
{
  bool  isRisingEdge;
  uint16_t a0a15;
  uint8_t d0d7;
  bool readNotWrite;
  bool isA0A15SetToOutput;
} CPUSTATE;

typedef struct 
{
  CPUSTATE cpuState;
} SYSTEMSTATE;

/** Mask for pins 0-11, 21 and 28 
 *  A0-A15 multiplexed by 3x 3-state octal bus transceiver 74LVC245APW (pins 0-7)
 *  Multiplexing pins 8,9,10 for output enable signal(OE) for U5, U6 and U7 (high= Z-state, low=enable)
 *  Pin 11 is used for data direction of U7 when OE=low. HIGH=Read from bus, LOW= Write to bus
 *  PIN 25,26,27 are for IRQ, RESET, NMI
 */
constexpr uint32_t pioMask_CPU = 0b00001110001000000000111111111111; 
constexpr uint32_t pioMaskData_U5_U6_U7 = 0b011111111; 
constexpr uint32_t pioMaskData_U5_U6_U7_RW = 0b100011111111; 
constexpr uint32_t pioMaskOE_U5_U6_U7 =  0b0000000000000000000011100000000; // Modify PIO PINs 8,9 and 10 (74LVC245APW U5, U6 and U7) only

constexpr uint32_t enableU5Only =  0b0000000000000000000011000000000; // set OE (active low) to low on U5 (PIO 8, A0-A7), others to high 
constexpr uint32_t enableU6Only =  0b0000000000000000000010100000000; // set OE (active low) to low on U6 (PIO 9, A8-A15), others to high  
constexpr uint32_t enableU7Only =  0b0000000000000000000001100000000; // set OE (active low) to low on U7 (PIO 10, D0-D7), others to high 
constexpr uint32_t disableU5U6U7 = 0b0000000000000000000011100000000; // set OE (active low) to high for Z-state of U5, U6 and U7

class RpPetra {
  
  public: 
    Logging *m_pLog;    
    VIC6569 *m_pVICII;  
    uint8_t *m_pRAM;
    CIA6526 *m_pCIA1;
    CIA6526 *m_pCIA2;
    Keyboard *m_pKeyboard;
  private:
    RP65C02 *m_pCPU;
    VideoOut *m_pVideoOut;
    uint8_t m_cpuAddr;

    void Enable_U5_only() { gpio_put_masked(pioMaskOE_U5_U6_U7, enableU5Only); };  
    void Enable_U6_only() { gpio_put_masked(pioMaskOE_U5_U6_U7, enableU6Only); };  
    void Enable_U7_only() { gpio_put_masked(pioMaskOE_U5_U6_U7, enableU7Only); };  
    void DisableBus()     { gpio_put_masked(pioMaskOE_U5_U6_U7, disableU5U6U7);}
    void ResetCPU();
    void ClockCPU(int counter);
    void PHI2(bool isRisingEdge) { gpio_put(CLK,isRisingEdge);}
    inline void WriteDataBus(uint8_t byte);
    inline void ReadCPUSignals(SYSTEMSTATE *pSystemState);
    inline bool IsIOVisible();
    inline bool IsBasicRomVisible();
    inline bool IsKernalRomVisible();
    inline bool IsCharRomVisible();

    void DumpScreen();
    
  public:
    bool m_screenUpdated;

    void Clk(bool isRisingEdge, SYSTEMSTATE *pSystemState);
    RpPetra(Logging *pLogging, RP65C02 *pCpu);
    void SignalIRQ(bool enable);
    void SignalNMI(bool enable);
    virtual ~RpPetra();
    void Reset();
    void UpdateScreen();
        
};

#endif