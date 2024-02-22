#ifndef _VIDEO_OUT_H
#define _VIDEO_OUT_H

extern const struct dvi_timing dvi_timing_340x240p_60hz;

extern  dvi_inst *g_pDVI; 

class VideoOut {

  public:
    VideoOut(Logging *pLog, RpPetra *pGlue, u_int8_t *pFrameBuffer);
    void Reset();
    void Start();
  private:
    Logging *m_pLog;
    

};

#endif