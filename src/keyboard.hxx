/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
 * The C-64 "physical" keyboard.
 * 
 */

#ifndef _KEYBOARD
#define _KEYBOARD

struct Keys{
  uint8_t row;
  uint8_t col;
};

/**
 * We use a list of keys in hope that the USB interface makes it possible
 * to distinguish multiple keys pressed at the same time.
*/
class Keyboard
{
  public:
    Keyboard (Logging *pLogging, RpPetra *pGlue);
    void OnKeyPressed(uint8_t row, uint8_t col);
    void OnKeyReleased(uint8_t row, uint8_t col);
    std::vector<Keys *> GetKeysPressed();

  private:
    std::vector<Keys *> m_keysPressed;
    Logging *m_pLog;
    RpPetra *m_pGlue;
};


#endif