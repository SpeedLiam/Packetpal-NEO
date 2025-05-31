#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// Forward declaration to avoid circular includes
class MenuSystem;

class ButtonHandler {
  public:
    ButtonHandler();
    void init();
    bool isButtonPressed(uint8_t pin);
    void checkButtons(MenuSystem* menuSystem);
    
    // Button pin definitions
    static const int BUTTON_A = 10;   // Selection button
    static const int BUTTON_B = 6;    // Back button
    static const int BUTTON_LEFT = 1;  // Left navigation
    static const int BUTTON_RIGHT = 8; // Right navigation
    static const int BUTTON_UP = 2;    // Up navigation
    static const int BUTTON_DOWN = 3;  // Down navigation
    
  private:
    unsigned long lastButtonPress;
    static const int DEBOUNCE_DELAY = 75;
};

#endif