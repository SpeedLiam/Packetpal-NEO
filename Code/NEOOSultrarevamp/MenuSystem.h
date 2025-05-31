#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

#include <Arduino.h>
#include "Display.h"
#include "ButtonHandler.h"

#define MAIN_MENU_COUNT 6  // Increased from 5 to 6 to add GPIO menu
#define SUB_MENU_COUNT 5
#define TRANSMISSION_SUB_MENU_COUNT 5

class MenuSystem {
  public:
    MenuSystem();
    void init(DisplayManager* displayManager, ButtonHandler* buttonHandler);
    void update();
    
    // Button handlers
    void handleSelectButton();
    void handleBackButton();
    void handleLeftButton();
    void handleRightButton();
    void handleUpButton();
    void handleDownButton();
    void handleBButton();  // New handler specifically for B button
    
    // Drawing methods (made public to allow direct access if needed)
    void drawMainMenu();
    void drawSubMenu();
    void drawFunctionScreen();  // New function to draw the function execution screen
    void drawTransmissionSubMenu();  // New method for transmission submenu
    
  private:
    DisplayManager* display;  // Using DisplayManager
    ButtonHandler* buttons;
    
    int currentMenu;      // 0 = main menu, 1 = submenu
    int mainMenuIndex;    // Current selected main menu option
    int subMenuIndex;     // Current selected submenu option
    int transmissionSubMenuIndex;  // New index for transmission submenu
    bool functionScreen;  // Flag to indicate function screen is active
    bool isTransmissionSubMenu;  // Flag to track transmission submenu
    
    // Main menu options
    const char* mainMenuOptions[MAIN_MENU_COUNT] = {
      "WIFI", "BLE", "INFRARED", "NEOKIN", "GPIO", "SETTINGS"
    };
    
    // Menu icons (using ASCII art as placeholder)
    const char* menuIcons[MAIN_MENU_COUNT] = {
      "-}", "{}", "} ~", "^.^", "<>", "#"
    };
    
    // Submenu options for each main menu
    const char* wifiSubMenuOptions[SUB_MENU_COUNT] = {
      "ATTACKS", "SCAN", "SELECT", "STATUS", "BACK"
    };
    
    const char* bleSubMenuOptions[SUB_MENU_COUNT] = {
      "ATTACKS", "SCAN", "STATUS", "CONFIG", "BACK"
    };
    
    const char* infraredSubMenuOptions[SUB_MENU_COUNT] = {
      "TRANSMISSION", "RECIEVE", "LIBRARY", "BOMBARDMENT", "BACK"
    };
    
    // New transmission submenu options
    const char* transmissionSubMenuOptions[TRANSMISSION_SUB_MENU_COUNT] = {
      "DIRECT SEND", 
      "REPEAT SEND", 
      "BURST SEND", 
      "ADAPTIVE SEND", 
      "BACK"
    };
    
    const char* neokinSubMenuOptions[SUB_MENU_COUNT] = {
      "STATUS", "VITALS", "LEVEL", "PLAY", "BACK"
    };
    
    // New GPIO submenu options
    const char* gpioSubMenuOptions[SUB_MENU_COUNT] = {
      "READ", "WRITE", "TOGGLE", "MONITOR", "BACK"
    };
    
    const char* settingsSubMenuOptions[SUB_MENU_COUNT] = {
      "GENERAL", "APPEARANCE", "DISPLAY", "OTHER", "BACK"
    };
    
    // Function execution
    void executeFunctionAction();  // New method to execute function actions
    
    // Module action functions
    void wifiScan();
    void wifiConnect();
    void wifiDeauth();
    void wifiRickroll();
    void wifiapSpam();
    void wifiSelect();
    void bleConnect();
    void bleSpamIos();
    void bleSpamAndroid();
    void bleSpamWindows();
    void bleSpamAll();
    void infraredTvbgone();
    void infraredReceive();
    void infraredSpam();
    void infraredPlayback();
    void infraredATKmenu();
    
    // Transmission specific methods
    void infraredDirectSend();
    void infraredRepeatSend();
    void infraredBurstSend();
    void infraredAdaptiveSend();
    
    // New GPIO action functions
    void gpioRead();
    void gpioWrite();
    void gpioToggle();
    void gpioMonitor();
};

#endif