#include <U8g2lib.h>
#include <IRremote.h>
#include <Wire.h>

// Initialize U8G2 display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Button definitions
#define BTN_LEFT D1
#define BTN_RIGHT D8
#define BTN_UP D9
#define BTN_SELECT D10
#define BTN_BACK D6
#define BTN_DOWN D8

// IR pins
#define IR_SEND_PIN 4
#define IR_RECEIVE_PIN 5

// Button debouncing
#define DEBOUNCE_DELAY 200
unsigned long lastButtonPress = 0;

// Menu constants
#define MAX_MENU_ITEMS 5
int currentMenuItem = 0;

// Menu items and icons arrays
const char* menuItems[] = {
    "Learn IR Code",
    "Send Last Code",
    "TV-B-Gone",
    "View Codes",
    "Bombardment"
};

const char* menuIcons[] = {
    ">",
    "T",
    "8",
    "?",
    "!"
};

// IR variables
uint32_t lastIRCode = 0;
uint8_t lastIRBits = 0;
decode_type_t lastIRProtocol = NEC;
uint16_t lastAddress = 0;
uint16_t lastCommand = 0;

// Function declarations
void displayMessage(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr, int duration = 1500);
void drawMenu();
void handleButtons();
void handleMenuSelection();
void learnIRCode();
void sendLastCode();
void tvBGoneMode();
void viewSavedCodes();
void irspam();
void showWelcomeScreen();

// Button state handling
bool isButtonPressed(uint8_t pin) {
    static unsigned long lastPressTime = 0;
    if (!digitalRead(pin)) {
        if (millis() - lastPressTime > DEBOUNCE_DELAY) {
            lastPressTime = millis();
            return true;
        }
    }
    return false;
}

void displayMessage(const char* line1, const char* line2, const char* line3, int duration) {
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(4, 5, line1);
    
    if(line2) {
        u8g2.drawStr(4, 20, line2);
    }
    if(line3) {
        u8g2.drawStr(4, 35, line3);
    }
    
    u8g2.sendBuffer();
    if(duration > 0) {
        delay(duration);
    }
}

void drawMenu() {
    u8g2.clearBuffer();
    
    // Draw title bar
    u8g2.setFont(u8g2_font_simple1_tr);
    u8g2.drawBox(0, 0, 128, 11);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 2, "NEOos");
    u8g2.setDrawColor(1);
    
    // Draw menu items
    for(int i = 0; i < MAX_MENU_ITEMS; i++) {
        int yPos = 14 + (i * 10);
        int iconX = 0;
        int iconY = yPos - 2;
        
        if(i == currentMenuItem) {
            u8g2.drawBox(0, yPos-1, 128, 11);
            u8g2.setDrawColor(0);
            u8g2.setFont(u8g2_font_iconquadpix_m_all);
            u8g2.drawStr(iconX, iconY, menuIcons[i]);
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(14, yPos, menuItems[i]);
            u8g2.setDrawColor(1);
        } else {
            u8g2.setFont(u8g2_font_iconquadpix_m_all);
            u8g2.drawStr(iconX, iconY, menuIcons[i]);
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(14, yPos, menuItems[i]);
        }
    }
    
    // Draw scrollbar
    int scrollBarHeight = 64 / MAX_MENU_ITEMS;
    int scrollBarY = 11 + (currentMenuItem * scrollBarHeight);
    u8g2.drawBox(126, scrollBarY, 2, scrollBarHeight);
    
    u8g2.sendBuffer();
}

void handleButtons() {
    if(isButtonPressed(BTN_UP)) {
        currentMenuItem = (currentMenuItem > 0) ? currentMenuItem - 1 : MAX_MENU_ITEMS - 1;
    }
    if(isButtonPressed(BTN_DOWN)) {
        currentMenuItem = (currentMenuItem < MAX_MENU_ITEMS - 1) ? currentMenuItem + 1 : 0;
    }
}

void handleMenuSelection() {
    switch(currentMenuItem) {
        case 0:
            learnIRCode();
            break;
        case 1:
            sendLastCode();
            break;
        case 2:
            tvBGoneMode();
            break;
        case 3:
            viewSavedCodes();
            break;
        case 4:
            irspam();
            break;
    }
}

void learnIRCode() {
    displayMessage("Point remote &", "press button...");
    
    while(!isButtonPressed(BTN_BACK)) {
        if(IrReceiver.decode()) {
            lastIRCode = IrReceiver.decodedIRData.decodedRawData;
            lastIRBits = IrReceiver.decodedIRData.numberOfBits;
            lastIRProtocol = IrReceiver.decodedIRData.protocol;
            lastAddress = IrReceiver.decodedIRData.address;
            lastCommand = IrReceiver.decodedIRData.command;
            
            char hexStr[20];
            sprintf(hexStr, "Value: 0x%08lX", lastIRCode);
            displayMessage("Code received!", hexStr, nullptr, 1500);
            
            IrReceiver.resume();
            break;
        }
    }
}

void sendLastCode() {
    if(lastIRCode == 0) {
        displayMessage("No code stored!");
        return;
    }
    
    displayMessage("Sending code...");
    
    switch(lastIRProtocol) {
        case NEC:
            IrSender.sendNEC(lastAddress, lastCommand, 0);
            break;
        case SAMSUNG:
            IrSender.sendSamsung(lastAddress, lastCommand, 0);
            break;
        case SONY:
            IrSender.sendSony(lastAddress, lastCommand, 0);
            break;
        case PANASONIC:
            IrSender.sendPanasonic(lastAddress, lastCommand, 0);
            break;
        default:
            IrSender.sendNEC(lastIRCode, lastIRBits);
    }
    
    delay(1000);
}

void tvBGoneMode() {
    displayMessage("TV-B-Gone Mode", "Hold SELECT to", "send all codes");
    
    while(!isButtonPressed(BTN_BACK)) {
        if(!digitalRead(BTN_SELECT)) {
            const struct {
                uint16_t addr;
                uint8_t cmd;
            } commonCodes[] = {
                {0x04, 0x08},  // LG
                {0x07, 0x02},  // Samsung
                {0x01, 0x15},  // Sony
                {0x80, 0x3D}   // Panasonic
            };
            
            for(const auto& code : commonCodes) {
                IrSender.sendNEC(code.addr, code.cmd, 0);
                delay(100);
            }
        }
        delay(50);
    }
}

void viewSavedCodes() {
    char hexStr1[20], hexStr2[20];
    sprintf(hexStr1, "Addr: 0x%04X", lastAddress);
    sprintf(hexStr2, "Cmd: 0x%02X", lastCommand);
    displayMessage("Last code info:", hexStr1, hexStr2, 2000);
}

void irspam() {
    displayMessage("IR Bombardment", "Press SELECT", "to begin");
    
    while(!isButtonPressed(BTN_SELECT)) {
        delay(50);
    }
    
    int totalCodes = 35;
    int currentCode = 0;
    int currentAngle = 0;
    
    while(currentCode < totalCodes && !isButtonPressed(BTN_BACK)) {
        uint16_t randomAddress = random(0, 0xFFFF);
        uint8_t randomCommand = random(0, 0xFF);
        
        IrSender.sendNEC(randomAddress, randomCommand, 0);
        
        // Update display
        u8g2.clearBuffer();
        char addrStr[20], cmdStr[20], progStr[20];
        sprintf(addrStr, "Addr: 0x%04X", randomAddress);
        sprintf(cmdStr, "Cmd: 0x%02X", randomCommand);
        sprintf(progStr, "Progress: %d/%d", currentCode + 1, totalCodes);
        
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 0, "IR Bombardment");
        u8g2.drawStr(0, 15, addrStr);
        u8g2.drawStr(0, 30, cmdStr);
        u8g2.drawStr(0, 45, progStr);
        
        // Draw spinner
        int centerX = 120, centerY = 50, radius = 6;
        int x = centerX + radius * cos(radians(currentAngle));
        int y = centerY + radius * sin(radians(currentAngle));
        u8g2.drawLine(centerX, centerY, x, y);
        
        u8g2.sendBuffer();
        
        currentCode++;
        currentAngle = (currentAngle + 45) % 360;
        delay(50);
    }
    
    displayMessage("Bombardment", "Complete!", nullptr, 1500);
}

void showWelcomeScreen() {
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);
    
    // Title
    u8g2.setFont(u8g2_font_doomalpha04_tr);
    u8g2.drawStr(35, 10, "NEOos");
    
    // Subtitle
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 30, "version fucktard");
    u8g2.drawStr(25, 45, "by steam");
    
    u8g2.sendBuffer();
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    
    // Initialize buttons
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    
    // Initialize display
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    
    // Initialize IR
    IrReceiver.begin(IR_RECEIVE_PIN);
    IrSender.begin(IR_SEND_PIN);
    
    // Show welcome screen
    showWelcomeScreen();
    delay(2000);
}

void loop() {
    handleButtons();
    drawMenu();
    
    if(isButtonPressed(BTN_SELECT)) {
        handleMenuSelection();
    }
    
    delay(50);
}
