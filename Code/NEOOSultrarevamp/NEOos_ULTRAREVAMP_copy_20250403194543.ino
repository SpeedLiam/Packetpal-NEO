#include <Arduino.h>
#include <Wire.h>
#include "Display.h"
#include "MenuSystem.h"
#include "ButtonHandler.h"

// Initialize display
DisplayManager display;

// Initialize menu system
MenuSystem menuSystem;

// Initialize button handler
ButtonHandler buttonHandler;

void setup() {
  Serial.begin(115200);
  
  // Wait a moment for device to initialize
  delay(500);
  
  // Initialize I2C with slower clock
  Wire.begin();
  Wire.setClock(100000);
  
  // Initialize components
  display.init();
  buttonHandler.init();
  menuSystem.init(&display, &buttonHandler);
  
  // Show main menu initially
  menuSystem.drawMainMenu();
}

void loop() {
  // Check button inputs
  buttonHandler.checkButtons(&menuSystem);
  
  // Update menu display
  menuSystem.update();
  
  delay(25);  // Standard delay for reasonable performance
}