/**
 * Written by Bernd Krekeler, Herne, Germany
 * 
 * The C-64 "physical" keyboard.
 * 
 */

#include "stdinclude.hxx"

Keyboard::Keyboard(Logging *pLogging, RpPetra *pGlue)
{

}

/*
    Signal that a key has been pressed.
    We will check if we can detect multiple different keypress events simultaneously. But this
    depends on the USB keyboard interface.
*/
void Keyboard::OnKeyPressed(uint8_t row, uint8_t col)
{

}

/**
 * Key has been released. 0,0=no key pressed.
*/
void Keyboard::OnKeyReleased(uint8_t row, uint8_t col)
{

}

std::vector<Keys> Keyboard::GetKeysPressed()
{
  return m_keysPressed;
}