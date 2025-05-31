#include "ButtonHandler.h"
#include "MenuSystem.h"

ButtonHandler::ButtonHandler() : lastButtonPress(0) {
  // Constructor
}

void ButtonHandler::init() {
  // Initialize buttons with internal pullup
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
}

bool ButtonHandler::isButtonPressed(uint8_t pin) {
  if (!digitalRead(pin)) {
    if (millis() - lastButtonPress > DEBOUNCE_DELAY) {
      lastButtonPress = millis();
      return true;
    }
  }
  return false;
}

void ButtonHandler::checkButtons(MenuSystem* menuSystem) {
  // Check A button (selection)
  if (isButtonPressed(BUTTON_A)) {
    menuSystem->handleSelectButton();
  }
  
  // Check B button for going back
  if (isButtonPressed(BUTTON_B)) {
    menuSystem->handleBackButton();
  }
  
  // Check left button
  if (isButtonPressed(BUTTON_LEFT)) {
    menuSystem->handleLeftButton();
  }
  
  // Check right button
  if (isButtonPressed(BUTTON_RIGHT)) {
    menuSystem->handleRightButton();
  }
  
  // Check UP button
  if (isButtonPressed(BUTTON_UP)) {
    menuSystem->handleUpButton();
  }
  
  // Check DOWN button
  if (isButtonPressed(BUTTON_DOWN)) {
    menuSystem->handleDownButton();
  }
}