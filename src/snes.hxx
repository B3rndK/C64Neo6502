/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

class SNES : public Joysticks
{
  public:
    SNES(Logging *pLog);
    virtual void Convert(uint8_t const * report, uint16_t len);
};