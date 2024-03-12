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

// We will make it easy to support different joysticks using a simple bitfield
struct JoystickStatus {
  uint8_t port:1; // 0=port 1, 1=port 2
  uint8_t up:1; // 0- pushed, 1=not pushed
  uint8_t down:1;
  uint8_t left:1;
  uint8_t right:1;
  uint8_t fire:1;
};
#define PRESSED 0
#define NOT_PRESSED 1

#define PORT_1 0
#define PORT_2 1

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