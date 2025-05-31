#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>

class DisplayManager {
  public:
    DisplayManager();
    void init();
    void clearBuffer();
    void sendBuffer();
    void drawRFrame(int x, int y, int width, int height, int radius);
    void drawStr(int x, int y, const char* text);
    void setFont(const uint8_t* font);
    int getStrWidth(const char* text);
    
    // Provide direct access to u8g2 for more complex operations
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C* getU8g2();

  private:
    // Use Software I2C with pins 7 (clock) and 6 (data)
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2;
};

#endif