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

// Menu items array
const char* menuItems[] = {
    "Learn IR Code",
    "Send Last Code",
    "TV-B-Gone",
    "View Codes",
    "Bombardment"
};

// IR variables
uint32_t lastIRCode = 0;
uint8_t lastIRBits = 0;
decode_type_t lastIRProtocol = NEC;
uint16_t lastAddress = 0;
uint16_t lastCommand = 0;

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

// Window drawing functions
void drawWindowFrame() {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawBox(0, 0, 128, 10);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(4, 2, "NEOos v.1.0");
    u8g2.drawStr(120, 1, "x");
    u8g2.drawStr(113, 1, "-");
    u8g2.setDrawColor(1);
}

void drawMenu() {
    u8g2.clearBuffer();
    drawWindowFrame();
    
    int startY = 14;
    for(int i = 0; i < MAX_MENU_ITEMS; i++) {
        int yPos = startY + (i * 10);
        
        if(i == currentMenuItem) {
            u8g2.setFont(u8g2_font_4x6_tf);
            u8g2.drawStr(4, yPos, ">");
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(12, yPos, menuItems[i]);
        } else {
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(12, yPos, menuItems[i]);
        }
    }
    
    u8g2.sendBuffer();
}

void displayMessage(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr, int duration = 1500) {
    u8g2.clearBuffer();
    drawWindowFrame();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(4, 14, line1);
    
    if(line2) {
        u8g2.drawStr(4, 26, line2);
    }
    if(line3) {
        u8g2.drawStr(4, 38, line3);
    }
    
    u8g2.sendBuffer();
    if(duration > 0) {
        delay(duration);
    }
}

// IR Functions
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

    const struct {
        uint16_t addr;
        uint8_t cmd;
    } commonCodes[] = {
        {0x04, 0x08},  // LG
        {0x07, 0x02},  // Samsung
        {0x01, 0x15},  // Sony
        {0x80, 0x3D},  // Panasonic
        {0x10, 0x01},  // Vizio
        {0x50, 0x10},  // Sharp
        {0x30, 0x05},  // Philips
        {0x20, 0x04},  // Toshiba
        {0x40, 0x03},  // Hitachi
        {0x60, 0x09},  // Mitsubishi
        {0x70, 0x07},  // JVC
        {0x90, 0x11},  // RCA
        {0xA0, 0x13},  // Sanyo
        {0xB0, 0x14},  // Akai
        {0xC0, 0x16},  // Benq
        {0xD0, 0x18},  // Funai
        {0xE0, 0x1A},  // Emerson
        {0xF0, 0x1C},  // Insignia
        {0x100, 0x1E}, // LG (another model)
        {0x110, 0x1F}, // Panasonic (another model)
        {0x120, 0x20}, // Sharp (another model)
        {0x130, 0x22}, // Samsung (another model)
        {0x140, 0x24}, // Sony (another model)
        {0x150, 0x26}, // Vizio (another model)
        {0x160, 0x28}, // Toshiba (another model)
        {0x170, 0x2A}, // Philips (another model)
        {0x180, 0x2C}, // LG (another model)
        {0x190, 0x2E}, // Samsung (another model)
        {0x1A0, 0x30}, // Sony (another model)
        {0x1B0, 0x32}, // Panasonic (another model)
        {0x1C0, 0x34}, // Vizio (another model)
        {0x1D0, 0x36}, // Sharp (another model)
        {0x1E0, 0x38}, // Mitsubishi (another model)
        {0x1F0, 0x3A}, // Toshiba (another model)
        {0x200, 0x3C}, // Akai (another model)
        {0x210, 0x3E}, // Insignia (another model)
        {0x220, 0x40}, // Funai (another model)
        {0x230, 0x42}, // RCA (another model)
        {0x240, 0x44}, // JVC (another model)
        {0x250, 0x46}, // Sharp (yet another model)
        {0x260, 0x48}, // Vizio (yet another model)
        {0x270, 0x4A}, // Panasonic (yet another model)
        {0x280, 0x4C}, // Sony (yet another model)
        {0x290, 0x4E}, // Samsung (yet another model)
        {0x2A0, 0x50}, // LG (yet another model)
        {0x2B0, 0x52}, // Mitsubishi (yet another model)
        {0x2C0, 0x54}, // Hitachi (another model)
        {0x2D0, 0x56}, // Toshiba (yet another model)
        {0x2E0, 0x58}, // Sharp (final model)
        {0x2F0, 0x5A}, // Vizio (final model)
        {0x300, 0x5C}, // Benq (another model)
        {0x310, 0x5E}, // Akai (final model)
        {0x320, 0x60}, // Funai (final model)
        {0x330, 0x62}, // RCA (final model)
        {0x340, 0x64}, // Insignia (final model)
        {0x350, 0x66}, // JVC (final model)
        {0x360, 0x68}, // Sanyo (final model)
        {0x370, 0x6A}, // Mitsubishi (final model)
        {0x380, 0x6C}, // Sony (final model)
        {0x390, 0x6E}  // LG (final model)
    };

    int totalCodes = sizeof(commonCodes) / sizeof(commonCodes[0]);

    while (!isButtonPressed(BTN_BACK)) {
        if (!digitalRead(BTN_SELECT)) {
            for (int i = 0; i < totalCodes; i++) {
                IrSender.sendNEC(commonCodes[i].addr, commonCodes[i].cmd, 0);

                // Calculate progress as a percentage
                int progress = map(i, 0, totalCodes - 1, 0, 100);

                // Clear the previous screen content
                u8g2.clearBuffer();

                // Draw the menu frame
                drawWindowFrame();

                // Display progress percentage, adjusted to avoid clipping
                u8g2.setFont(u8g2_font_minicute_tr);
                u8g2.setCursor(4, 20); // Adjusted cursor to avoid clipping
                u8g2.print("Progress: ");
                u8g2.print(progress); // Show the percentage
                u8g2.print("%");
                u8g2.sendBuffer();

                delay(100); // Delay to simulate the code sending
            }

            // Once all codes are sent, display "All codes sent"
            u8g2.clearBuffer(); // Clear the previous content
            drawWindowFrame(); // Redraw the frame
            u8g2.setFont(u8g2_font_minicute_tr);
            u8g2.setCursor(4, 20); // Adjusted cursor to avoid clipping
            u8g2.print("All codes sent");
            u8g2.sendBuffer();
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
        drawWindowFrame();
        
        char addrStr[20], cmdStr[20], progStr[20];
        sprintf(addrStr, "Addr: 0x%04X", randomAddress);
        sprintf(cmdStr, "Cmd: 0x%02X", randomCommand);
        sprintf(progStr, "Progress: %d/%d", currentCode + 1, totalCodes);
        
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(4, 14, "IR Bombardment");
        u8g2.drawStr(4, 26, addrStr);
        u8g2.drawStr(4, 38, cmdStr);
        u8g2.drawStr(4, 50, progStr);
        
        u8g2.sendBuffer();
        
        currentCode++;
        currentAngle = (currentAngle + 45) % 360;
        delay(50);
    }
    
    displayMessage("Bombardment", "Complete!", nullptr, 1500);
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

void showWelcomeScreen() {
    u8g2.clearBuffer();
    drawWindowFrame();
    
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(4, 20, "NEOos");
    u8g2.drawStr(4, 35, "Version 1.0");
    u8g2.drawStr(4, 50, "Press SELECT...");
    
    u8g2.sendBuffer();
    delay(2000);
}

void setup() {
    Serial.begin(115200);
    
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    
    IrReceiver.begin(IR_RECEIVE_PIN);
    IrSender.begin(IR_SEND_PIN);
    
    showWelcomeScreen();
}

void loop() {
    handleButtons();
    drawMenu();
    
    if(isButtonPressed(BTN_SELECT)) {
        handleMenuSelection();
    }
    
    delay(50);
}
