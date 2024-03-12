/**
 * For easy converting any kind of joystick towards C-64
 * Written by Bernd Krekeler, Herne, Germany.
*/
class Joysticks
{
  public:
    Joysticks(Logging *pLogging, uint8_t port=PORT_2);
    virtual ~Joysticks();
    // Only method to be implemented for more joysticks.
    virtual void Convert(uint8_t const * report, uint16_t len) = 0;
    // Add additional helper methods here, if required.
    void ClearButtonStatus(void);
    JoystickStatus m_status;
  protected:
    Logging *m_pLog;
    uint8_t m_port;
    
};