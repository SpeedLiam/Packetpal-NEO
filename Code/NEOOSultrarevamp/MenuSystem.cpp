#include "MenuSystem.h"

// Constructor - initialize new variables
MenuSystem::MenuSystem()
  : currentMenu(0), mainMenuIndex(0), subMenuIndex(0), 
    transmissionSubMenuIndex(0), functionScreen(false), 
    isTransmissionSubMenu(false), display(nullptr), buttons(nullptr) {
  // Constructor
}

void MenuSystem::init(DisplayManager* displayManager, ButtonHandler* buttonHandler) {
  display = displayManager;
  buttons = buttonHandler;
}

// Update the update method to handle the new transmission submenu
void MenuSystem::update() {
  static bool lastFunctionScreen = false;
  
  // Log state transitions for debugging
  if (functionScreen != lastFunctionScreen) {
    if (functionScreen) {
      Serial.println("Transition: Entered function screen");
    } else {
      Serial.println("Transition: Exited function screen");
    }
    lastFunctionScreen = functionScreen;
  }

  // Redraw the current menu
  if (currentMenu == 0) {
    drawMainMenu();
  } else if (isTransmissionSubMenu) {
    drawTransmissionSubMenu();
  } else if (functionScreen) {
    Serial.println("Drawing function screen");
    drawFunctionScreen();
  } else {
    Serial.println("Drawing submenu screen");
    drawSubMenu();
  }
}

// New method to draw transmission submenu
void MenuSystem::drawTransmissionSubMenu() {
  display->clearBuffer();
  
  // Draw frame using RFrame for rounded corners
  display->drawRFrame(0, 0, 128, 64, 4);
  display->drawRFrame(0, 0, 128, 12, 4);
  
  // Draw context-specific title
  display->setFont(u8g2_font_4x6_tr);
  display->drawStr(5, 9, ":// INFRARED TRANSMISSION");
  display->drawStr(110, 8, "- X");
  
  // Draw submenu items in vertical list
  int startY = 24;
  for (int i = 0; i < TRANSMISSION_SUB_MENU_COUNT; i++) {
    int yPos = startY + (i * 10);
    
    if (i == transmissionSubMenuIndex) {
      // Draw selected item with cursor
      display->setFont(u8g2_font_4x6_tf);
      display->drawStr(4, yPos, ">");
      
      display->setFont(u8g2_font_6x10_tf);
      display->drawStr(12, yPos, transmissionSubMenuOptions[i]);
    } else {
      // Draw unselected item
      display->setFont(u8g2_font_6x10_tf);
      display->drawStr(12, yPos, transmissionSubMenuOptions[i]);
    }
  }
  
  display->sendBuffer();
}

// Modify existing button handlers to support the new submenu
void MenuSystem::handleSelectButton() {
  Serial.println("SELECT button pressed");
  
  if (currentMenu == 0) {
    // Enter submenu from main menu
    Serial.println("Entering submenu from main menu");
    currentMenu = 1;
    subMenuIndex = 0;
    functionScreen = false;
  } else if (isTransmissionSubMenu) {
    // Handle transmission submenu selection
    if (transmissionSubMenuOptions[transmissionSubMenuIndex] == "BACK") {
      // Exit transmission submenu
      isTransmissionSubMenu = false;
      currentMenu = 1;
    } else {
      // Execute specific transmission method
      Serial.print("Selected transmission option: ");
      Serial.println(transmissionSubMenuOptions[transmissionSubMenuIndex]);
      
      // Execute corresponding transmission method
      switch(transmissionSubMenuIndex) {
        case 0: 
          infraredDirectSend();
          break;
        case 1:
          infraredRepeatSend();
          break;
        case 2:
          infraredBurstSend();
          break;
        case 3:
          infraredAdaptiveSend();
          break;
      }
    }
  } else if (functionScreen) {
    // Execute function when in function screen
    Serial.println("Executing function from function screen");
    executeFunctionAction();
  } else {
    // Existing submenu selection logic
    const char** currentSubMenuOptions;
    
    switch (mainMenuIndex) {
      case 0: currentSubMenuOptions = wifiSubMenuOptions; break;
      case 1: currentSubMenuOptions = bleSubMenuOptions; break;
      case 2: currentSubMenuOptions = infraredSubMenuOptions; break;
      case 3: currentSubMenuOptions = neokinSubMenuOptions; break;
      case 4: currentSubMenuOptions = gpioSubMenuOptions; break;
      case 5: currentSubMenuOptions = settingsSubMenuOptions; break;
      default: currentSubMenuOptions = wifiSubMenuOptions;
    }
    
    Serial.print("Selected submenu option: ");
    Serial.println(currentSubMenuOptions[subMenuIndex]);
    
    // Check if "BACK" option is selected
    if (strcmp(currentSubMenuOptions[subMenuIndex], "BACK") == 0) {
      Serial.println("BACK option selected, returning to main menu");
      currentMenu = 0; // Return to main menu
    } else {
      // Enter function screen
      Serial.println("Entering function screen");
      functionScreen = true;
    }
  }
}

void MenuSystem::handleBackButton() {
  Serial.println("BACK button pressed");
  
  if (isTransmissionSubMenu) {
    Serial.println("Exiting transmission submenu");
    isTransmissionSubMenu = false;
    currentMenu = 1;
  } else if (functionScreen) {
    Serial.println("Exiting function screen to submenu");
    functionScreen = false;
  } else if (currentMenu == 1) {
    Serial.println("Exiting submenu to main menu");
    currentMenu = 0;
  }
}

void MenuSystem::handleBButton() {
  Serial.println("B button pressed");
  
  if (isTransmissionSubMenu) {
    Serial.println("B button: Exiting transmission submenu");
    isTransmissionSubMenu = false;
    currentMenu = 1;
  } else if (functionScreen) {
    Serial.println("B button: Exiting function screen to submenu");
    functionScreen = false;
  } else if (currentMenu == 1) {
    Serial.println("B button: Exiting submenu to main menu");
    currentMenu = 0;
  }
  
  // Hack: force a direct update to ensure the display refreshes
  if (currentMenu == 0) {
    drawMainMenu();
  } else if (functionScreen == false && currentMenu == 1) {
    drawSubMenu();
  }
}

// Update other navigation methods to support transmission submenu
void MenuSystem::handleLeftButton() {
  if (currentMenu == 0) {
    // Main menu navigation
    mainMenuIndex = (mainMenuIndex + MAIN_MENU_COUNT - 1) % MAIN_MENU_COUNT;
  } else if (isTransmissionSubMenu) {
    // Transmission submenu navigation
    transmissionSubMenuIndex = (transmissionSubMenuIndex + TRANSMISSION_SUB_MENU_COUNT - 1) % TRANSMISSION_SUB_MENU_COUNT;
  } else if (!functionScreen) {
    // Regular submenu navigation
    subMenuIndex = (subMenuIndex + SUB_MENU_COUNT - 1) % SUB_MENU_COUNT;
  }
}

void MenuSystem::handleRightButton() {
  if (currentMenu == 0) {
    // Main menu navigation
    mainMenuIndex = (mainMenuIndex + 1) % MAIN_MENU_COUNT;
  } else if (isTransmissionSubMenu) {
    // Transmission submenu navigation
    transmissionSubMenuIndex = (transmissionSubMenuIndex + 1) % TRANSMISSION_SUB_MENU_COUNT;
  } else if (!functionScreen) {
    // Regular submenu navigation
    subMenuIndex = (subMenuIndex + 1) % SUB_MENU_COUNT;
  }
}

void MenuSystem::handleUpButton() {
  if (currentMenu == 0) {
    mainMenuIndex = (mainMenuIndex + MAIN_MENU_COUNT - 1) % MAIN_MENU_COUNT;
  } else if (isTransmissionSubMenu) {
    transmissionSubMenuIndex = (transmissionSubMenuIndex > 0) 
      ? transmissionSubMenuIndex - 1 
      : TRANSMISSION_SUB_MENU_COUNT - 1;
  } else if (!functionScreen) {
    subMenuIndex = (subMenuIndex > 0) ? subMenuIndex - 1 : SUB_MENU_COUNT - 1;
  }
}

void MenuSystem::handleDownButton() {
  if (currentMenu == 0) {
    mainMenuIndex = (mainMenuIndex + 1) % MAIN_MENU_COUNT;
  } else if (isTransmissionSubMenu) {
    transmissionSubMenuIndex = (transmissionSubMenuIndex < TRANSMISSION_SUB_MENU_COUNT - 1) 
      ? transmissionSubMenuIndex + 1 
      : 0;
  } else if (!functionScreen) {
    subMenuIndex = (subMenuIndex < SUB_MENU_COUNT - 1) ? subMenuIndex + 1 : 0;
  }
}

// Complete the existing methods from the original implementation

void MenuSystem::drawMainMenu() {
  display->clearBuffer();
  
  // Draw frame using RFrame for rounded corners
  display->drawRFrame(0, 0, 128, 64, 4);
  display->drawRFrame(0, 0, 128, 12, 4);
  
  // Draw title
  display->setFont(u8g2_font_4x6_tr);
  display->drawStr(4, 9, "NEOos V.1.0");
  display->drawStr(110, 8, "- X");
  
  // Draw navigation indicators
  display->drawStr(119, 55, ">");
  display->drawStr(5, 55, "<");
  
  // Draw selected menu item
  display->setFont(u8g2_font_profont17_tr);
  int textWidth = display->getStrWidth(mainMenuOptions[mainMenuIndex]);
  display->drawStr(64 - (textWidth / 2), 58, mainMenuOptions[mainMenuIndex]);
  
  // Draw menu icon centered
  const char* icon = menuIcons[mainMenuIndex];
  int iconWidth = display->getStrWidth(icon);
  int iconX = (128 - iconWidth) / 2; // Center the icon
  display->drawStr(iconX, 35, icon);
  
  display->sendBuffer();
}

void MenuSystem::drawSubMenu() {
  display->clearBuffer();
  
  // Draw frame using RFrame for rounded corners
  display->drawRFrame(0, 0, 128, 64, 4);
  display->drawRFrame(0, 0, 128, 12, 4);
  
  // Get current submenu options
  const char** currentSubMenuOptions;
  
  switch (mainMenuIndex) {
    case 0: currentSubMenuOptions = wifiSubMenuOptions; break;
    case 1: currentSubMenuOptions = bleSubMenuOptions; break;
    case 2: currentSubMenuOptions = infraredSubMenuOptions; break;
    case 3: currentSubMenuOptions = neokinSubMenuOptions; break;
    case 4: currentSubMenuOptions = gpioSubMenuOptions; break;   // New GPIO menu
    case 5: currentSubMenuOptions = settingsSubMenuOptions; break;
    default: currentSubMenuOptions = wifiSubMenuOptions;
  }
  
  // Draw context-specific title
  display->setFont(u8g2_font_4x6_tr);
  char contextTitle[20];
  snprintf(contextTitle, sizeof(contextTitle), ":// %s", currentSubMenuOptions[subMenuIndex]);
  display->drawStr(5, 9, contextTitle);
  
  display->drawStr(110, 8, "- X");
  
  // Draw submenu items in vertical list
  int startY = 24;
  for (int i = 0; i < SUB_MENU_COUNT; i++) {
    int yPos = startY + (i * 10);
    
    if (i == subMenuIndex) {
      // Draw selected item with cursor
      display->setFont(u8g2_font_4x6_tf);
      display->drawStr(4, yPos, ">");
      
      display->setFont(u8g2_font_6x10_tf);
      display->drawStr(12, yPos, currentSubMenuOptions[i]);
    } else {
      // Draw unselected item
      display->setFont(u8g2_font_6x10_tf);
      display->drawStr(12, yPos, currentSubMenuOptions[i]);
    }
  }
  
  display->sendBuffer();
}

void MenuSystem::drawFunctionScreen() {
  display->clearBuffer();
  
  // Draw frames
  display->drawRFrame(0, 0, 128, 64, 4);
  display->drawRFrame(0, 0, 128, 10, 3);
  
  // Get current submenu options and main menu context
  const char** currentSubMenuOptions;
  const char* mainMenuName = mainMenuOptions[mainMenuIndex];
  
  switch (mainMenuIndex) {
    case 0: currentSubMenuOptions = wifiSubMenuOptions; break;
    case 1: currentSubMenuOptions = bleSubMenuOptions; break;
    case 2: currentSubMenuOptions = infraredSubMenuOptions; break;
    case 3: currentSubMenuOptions = neokinSubMenuOptions; break;
    case 4: currentSubMenuOptions = gpioSubMenuOptions; break;   // New GPIO menu
    case 5: currentSubMenuOptions = settingsSubMenuOptions; break;
    default: currentSubMenuOptions = wifiSubMenuOptions;
  }
  
  const char* functionName = currentSubMenuOptions[subMenuIndex];
  
  // Draw header elements with actual function name
  display->setFont(u8g2_font_4x6_tr);
  display->drawStr(109, 7, "- X");
  
  // Show main menu context in the title
  char titleName[30];
  snprintf(titleName, sizeof(titleName), "%s:%s", mainMenuName, functionName);
  display->drawStr(8, 8, titleName);
  
  
  // Draw status indicator with context-specific info
  display->setFont(u8g2_font_6x12_tr);
  if (mainMenuIndex == 0) {
    display->drawStr(111, 23, "-}");
  } else if (mainMenuIndex == 1) {
    display->drawStr(111, 23, "{}");
  } else if (mainMenuIndex == 2) {
    display->drawStr(111, 23, "} ~");
  } else if (mainMenuIndex == 3) {
    display->drawStr(111, 23, "^.^");
  } else if (mainMenuIndex == 4) {
    display->drawStr(111, 23, "<>");  // New GPIO status display
  } else {
    display->drawStr(111, 23, "#");
  }
  
  display->sendBuffer();
}

void MenuSystem::executeFunctionAction() {
  // Process specific submenu action
  Serial.print("Executing function: ");
  
  const char** currentSubMenuOptions;
  
  switch (mainMenuIndex) {
    case 0: currentSubMenuOptions = wifiSubMenuOptions; break;
    case 1: currentSubMenuOptions = bleSubMenuOptions; break;
    case 2: currentSubMenuOptions = infraredSubMenuOptions; break;
    case 3: currentSubMenuOptions = neokinSubMenuOptions; break;
    case 4: currentSubMenuOptions = gpioSubMenuOptions; break;   // New GPIO menu
    case 5: currentSubMenuOptions = settingsSubMenuOptions; break;
    default: currentSubMenuOptions = wifiSubMenuOptions;
  }
  
  Serial.println(currentSubMenuOptions[subMenuIndex]);
  
  // Call appropriate action function based on menu and submenu selection
  if (mainMenuIndex == 0) { // WIFI
    if (subMenuIndex == 0) wifiScan();
    if (subMenuIndex == 1) wifiConnect();
  } else if (mainMenuIndex == 1) { // BLE
    if (subMenuIndex == 0) bleConnect();
  } else if (mainMenuIndex == 2) { // INFRARED
    switch(subMenuIndex) {
      case 0: // Transmission
        infraredATKmenu(); 
        break;
      case 1: 
        infraredReceive(); 
        break;
      case 2: 
        // Library function (if you have one)
        break;
      case 3: 
        // Bombardment function
        break;
    }
  } else if (mainMenuIndex == 4) { // GPIO
    if (subMenuIndex == 0) gpioRead();
    if (subMenuIndex == 1) gpioWrite();
    if (subMenuIndex == 2) gpioToggle();
    if (subMenuIndex == 3) gpioMonitor();
  }
  // Add more menu actions as needed
}

// Placeholder implementations for other methods
void MenuSystem::wifiScan() {
  Serial.println("WIFI SCAN");
  // Implementation for WiFi scan
}

void MenuSystem::wifiConnect() {
  Serial.println("WIFI CONNECT");
  // Implementation for WiFi connect
}

void MenuSystem::bleConnect() {
  Serial.println("BLE CONNECT");
  // Implementation for BLE connect
}

void MenuSystem::infraredReceive() {
  Serial.println("INFRARED RECEIVE");
  // Implementation for infrared receive
}

void MenuSystem::infraredATKmenu() {
  Serial.println("INFRARED ATK MENU");
  
  // Directly enter transmission mode without additional button press
  currentMenu = 1;  // Ensure we're in submenu mode
  isTransmissionSubMenu = true;
  transmissionSubMenuIndex = 0;
  
  // Draw the transmission submenu immediately
  drawTransmissionSubMenu();
}

void MenuSystem::infraredDirectSend() {
  Serial.println("Executing Infrared Direct Send");
  
  // Persistent display until B is pressed
  bool exitMode = false;
  
  while (!exitMode) {
    display->clearBuffer();
    display->drawRFrame(0, 0, 128, 64, 4);
    display->setFont(u8g2_font_6x10_tf);
    
    // Title
    display->drawStr(10, 15, "DIRECT SEND MODE");
    
    // Status or additional info
    display->drawStr(10, 30, "Transmitting...");
    
    // Exit instructions
    display->drawStr(10, 50, "Press B to exit");
    
    display->sendBuffer();
    
    // Check for exit condition (B button)
    if (buttons->isButtonPressed(BUTTON_B)) {
      exitMode = true;
      delay(200);  // Debounce delay
    }
    
    // Optional: Add actual transmission logic here
    // For now, just a placeholder
    delay(100);
  }
  
  // Return to transmission submenu
  isTransmissionSubMenu = true;
  transmissionSubMenuIndex = 0;
  drawTransmissionSubMenu();
}

void MenuSystem::infraredRepeatSend() {
  Serial.println("Executing Infrared Repeat Send");
  
  // Persistent display until B is pressed
  bool exitMode = false;
  int repeatCount = 0;
  
  while (!exitMode) {
    display->clearBuffer();
    display->drawRFrame(0, 0, 128, 64, 4);
    display->setFont(u8g2_font_6x10_tf);
    
    // Title
    display->drawStr(10, 15, "REPEAT SEND MODE");
    
    // Repeat count
    char repeatStr[30];
    snprintf(repeatStr, sizeof(repeatStr), "Repeats: %d", repeatCount);
    display->drawStr(10, 30, repeatStr);
    
    // Exit instructions
    display->drawStr(10, 50, "Press B to exit");
    
    display->sendBuffer();
    
    // Check for exit condition (B button)
    if (buttons->isButtonPressed(BUTTON_B)) {
      exitMode = true;
      delay(200);  // Debounce delay
    }
    
    // Optional: Add actual transmission logic here
    // Increment repeat count as an example
    repeatCount++;
    
    delay(100);
  }
  
  // Return to transmission submenu
  isTransmissionSubMenu = true;
  transmissionSubMenuIndex = 0;
  drawTransmissionSubMenu();
}

void MenuSystem::infraredBurstSend() {
  Serial.println("Executing Infrared Burst Send");
  
  // Persistent display until B is pressed
  bool exitMode = false;
  int burstCount = 0;
  
  while (!exitMode) {
    display->clearBuffer();
    display->drawRFrame(0, 0, 128, 64, 4);
    display->setFont(u8g2_font_6x10_tf);
    
    // Title
    display->drawStr(10, 15, "BURST SEND MODE");
    
    // Burst count
    char burstStr[30];
    snprintf(burstStr, sizeof(burstStr), "Burst Count: %d", burstCount);
    display->drawStr(10, 30, burstStr);
    
    // Exit instructions
    display->drawStr(10, 50, "Press B to exit");
    
    display->sendBuffer();
    
    // Check for exit condition (B button)
    if (buttons->isButtonPressed(BUTTON_B)) {
      exitMode = true;
      delay(200);  // Debounce delay
    }
    
    // Optional: Add actual burst transmission logic here
    // Increment burst count as an example
    burstCount++;
    
    delay(100);
  }
  
  // Return to transmission submenu
  isTransmissionSubMenu = true;
  transmissionSubMenuIndex = 0;
  drawTransmissionSubMenu();
}

void MenuSystem::infraredAdaptiveSend() {
  Serial.println("Executing Infrared Adaptive Send");
  
  // Persistent display until B is pressed
  bool exitMode = false;
  int adaptiveLevel = 0;
  
  while (!exitMode) {
    display->clearBuffer();
    display->drawRFrame(0, 0, 128, 64, 4);
    display->setFont(u8g2_font_6x10_tf);
    
    // Title
    display->drawStr(10, 15, "ADAPTIVE SEND MODE");
    
    // Adaptive level
    char adaptiveStr[30];
    snprintf(adaptiveStr, sizeof(adaptiveStr), "Adaptive Level: %d", adaptiveLevel);
    display->drawStr(10, 30, adaptiveStr);
    
    // Exit instructions
    display->drawStr(10, 50, "Press B to exit");
    
    display->sendBuffer();
    
    // Check for exit condition (B button)
    if (buttons->isButtonPressed(BUTTON_B)) {
      exitMode = true;
      delay(200);  // Debounce delay
    }
    
    // Optional: Add actual adaptive transmission logic here
    // Increment adaptive level as an example
    adaptiveLevel = (adaptiveLevel + 1) % 10;
    
    delay(100);
  }
  
  // Return to transmission submenu
  isTransmissionSubMenu = true;
  transmissionSubMenuIndex = 0;
  drawTransmissionSubMenu();
}

// GPIO function implementations
void MenuSystem::gpioRead() {
  Serial.println("GPIO READ");
  
  // Specific digital pins to monitor: 6, 1, 9, 7, 8
  const int pinCount = 5;
  const int digitalPins[pinCount] = {6, 1, 9, 7, 8};
  
  display->clearBuffer();
  display->drawRFrame(0, 0, 128, 64, 4);
  display->setFont(u8g2_font_6x10_tf);
  
  // Read and display GPIO pin states
  display->drawStr(10, 15, "GPIO PIN STATES:");
  
  // Read the specific pins
  for (int i = 0; i < pinCount; i++) {
    char pinInfo[20];
    int pinNumber = digitalPins[i];
    int pinState = digitalRead(pinNumber);
    snprintf(pinInfo, sizeof(pinInfo), "PIN %d: %s", pinNumber, pinState == HIGH ? "HIGH" : "LOW");
    display->drawStr(10, 25 + (i * 10), pinInfo);
  }
  
  display->drawStr(10, 60, "Press B to return");
  display->sendBuffer();
}

void MenuSystem::gpioWrite() {
  Serial.println("GPIO WRITE");
  
  // Specific digital pins: 6, 1, 9, 7, 8
  const int pinCount = 5;
  const int digitalPins[pinCount] = {6, 1, 9, 7, 8};
  static int selectedPinIndex = 0;
  
  display->clearBuffer();
  display->drawRFrame(0, 0, 128, 64, 4);
  display->setFont(u8g2_font_6x10_tf);
  
  display->drawStr(10, 15, "GPIO WRITE MODE");
  
  // Display the currently selected pin
  char selectedPin[30];
  snprintf(selectedPin, sizeof(selectedPin), "Selected: PIN %d (%s)", 
          digitalPins[selectedPinIndex], 
          digitalRead(digitalPins[selectedPinIndex]) == HIGH ? "HIGH" : "LOW");
  display->drawStr(10, 30, selectedPin);
  
  display->drawStr(10, 40, "Use UP/DOWN to select pin");
  display->drawStr(10, 50, "Use A to toggle state");
  display->drawStr(10, 60, "Press B to return");
  
  display->sendBuffer();
}

void MenuSystem::gpioToggle() {
  Serial.println("GPIO TOGGLE");
  
  // Specific digital pins: 6, 1, 9, 7, 8
  const int pinCount = 5;
  const int digitalPins[pinCount] = {6, 1, 9, 7, 8};
  
  display->clearBuffer();
  display->drawRFrame(0, 0, 128, 64, 4);
  display->setFont(u8g2_font_6x10_tf);
  
  display->drawStr(10, 15, "GPIO TOGGLE MODE");
  display->drawStr(10, 25, "Toggling all outputs...");
  
  // Toggle each specified pin
  for (int i = 0; i < pinCount; i++) {
    int pinNumber = digitalPins[i];
    int currentState = digitalRead(pinNumber);
    digitalWrite(pinNumber, !currentState);  // Toggle the state
    
    // Display toggle result for each pin
    char pinInfo[20];
    snprintf(pinInfo, sizeof(pinInfo), "PIN %d: %s -> %s", 
            pinNumber, 
            currentState == HIGH ? "HIGH" : "LOW", 
            !currentState == HIGH ? "HIGH" : "LOW");
    display->drawStr(10, 35 + (i * 8), pinInfo);
  }
  
  display->drawStr(10, 60, "Press B to return");
  display->sendBuffer();
}

void MenuSystem::gpioMonitor() {
  Serial.println("GPIO MONITOR");
  
  // Specific digital pins: 6, 1, 9, 7, 8
  const int pinCount = 5;
  const int digitalPins[pinCount] = {6, 1, 9, 7, 8};
  
  display->clearBuffer();
  display->drawRFrame(0, 0, 128, 64, 4);
  display->setFont(u8g2_font_6x10_tf);
  
  display->drawStr(10, 15, "GPIO MONITOR (LIVE)");
  
  // Show real-time state for specific pins
  for (int i = 0; i < pinCount; i++) {
    char pinInfo[20];
    int pinNumber = digitalPins[i];
    int pinState = digitalRead(pinNumber);
    
    // Draw pin number and state
    snprintf(pinInfo, sizeof(pinInfo), "PIN %d: %s", pinNumber, pinState == HIGH ? "HIGH" : "LOW");
    display->drawStr(10, 25 + (i * 10), pinInfo);
    
    // Draw visual indicator using text instead of graphics
    if (pinState == HIGH) {
      display->drawStr(100, 25 + (i * 10), "[ON]");
    } else {
      display->drawStr(100, 25 + (i * 10), "[OFF]");
    }
  }
  
  display->drawStr(10, 60, "Press B to return");
  display->sendBuffer();
}