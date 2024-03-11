/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
*/

class CompetitionPro : public Joysticks
{
  public:
    CompetitionPro(Logging *pLog);
    virtual void Convert(uint8_t const * report, uint16_t len);
};