
#ifndef VIC6569_HXX_
#define VIC6569_HXX_

#define CLOCKS_PER_HLINE 63
#define NUM_OF_VLINES_PAL 312 // 0 based, 311 is last line
#define LAST_VLINE_PAL 311 
#define END_SCANLINE_UPPER_BORDER_PAL 50
#define START_SCANLINE_LOWER_BORDER_PAL 251

class RpPetra;
 
class VIC6569 {
  
  private:  
    Logging *m_pLog;
    RpPetra *m_pGlue; 
    uint64_t m_i64Clks;
    uint8_t m_registerSetWrite[0x2f];
    uint16_t m_currentScanLine;
    uint8_t *m_pFrameBuffer;

    void UpdateFrameBuffer();
    void HandleTextMode(bool multicolor);
    void HandleHiresModes(bool multicolor);
    void HandleStandardBitmapMode();
    void HandleMulticolorBitmapMode();
    void HandleStandardTextMode();
    void HandleMulticolorTextMode();
    void HandleExtendedColorMode();
    uint16_t GetVideoRamStartAddr(bool bitmapMode);
    inline uint16_t GetTextModeCharRamAddrOffset();
    inline uint16_t GetVideoRamAddrOffset();
  
  public:
    VIC6569(Logging *pLogging, RpPetra *pGlue);
    virtual ~VIC6569();
    void Reset();
    void Clk();
    void WriteRegister(uint8_t reg, uint8_t value);
    inline uint8_t *GetFrameBuffer() { return m_pFrameBuffer;};
    uint8_t m_registerSetRead[0x2f];    
};

#endif