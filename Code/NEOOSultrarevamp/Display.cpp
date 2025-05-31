#include "Display.h"

DisplayManager::DisplayManager() 
  : u8g2(U8G2_R0, /* clock=*/ 7, /* data=*/ 6, /* reset=*/ U8X8_PIN_NONE) {
  // Constructor
}

void DisplayManager::init() {
  u8g2.begin();
}

void DisplayManager::clearBuffer() {
  u8g2.clearBuffer();
}

void DisplayManager::sendBuffer() {
  u8g2.sendBuffer();
}

void DisplayManager::drawRFrame(int x, int y, int width, int height, int radius) {
  u8g2.drawRFrame(x, y, width, height, radius);
}

void DisplayManager::drawStr(int x, int y, const char* text) {
  u8g2.drawStr(x, y, text);
}

void DisplayManager::setFont(const uint8_t* font) {
  u8g2.setFont(font);
}

int DisplayManager::getStrWidth(const char* text) {
  return u8g2.getStrWidth(text);
}

U8G2_SSD1306_128X64_NONAME_F_SW_I2C* DisplayManager::getU8g2() {
  return &u8g2;
}