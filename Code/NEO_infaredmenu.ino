#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <IRremote.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Pin definitions
const int IR_RECEIVE_PIN = 3;
const int IR_SEND_PIN = 2;

// Button pins
const int BTN_LEFT = 1;
const int BTN_RIGHT = 8;
const int BTN_UP = 9;
const int BTN_SELECT = 10;
const int BTN_BACK = 6;

// Display initialization
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// IR setup
IRrecv irrecv(IR_RECEIVE_PIN);
IRsend irsend(IR_SEND_PIN);
decode_results results;

// Menu variables
int currentMenuItem = 0;
const int MENU_ITEMS = 5;
String menuItems[] = {"Record IR", "Play Last", "Show Code", "Clear Memory", "Power Codes"};

// IR code storage
unsigned long lastIRCode = 0;
int irProtocol = 0;
int irBits = 0;

// Common power codes for different protocols
const unsigned long POWER_CODES[] PROGMEM = {
    0xE0E040BF,  // Samsung
    0x20DF10EF,  // LG
    0x4BB620DF,  // Sony
    0x2FD48B7,   // Philips
    0x1FE48B7,   // Panasonic
    0xC1AA09F6,  // Sharp
    0x40040D,    // Toshiba
    0x4FB4AB5,   // JVC
};

const int NUM_CODES = sizeof(POWER_CODES) / sizeof(POWER_CODES[0]);
bool isTransmitting = false;
int currentCode = 0;

// Button debounce variables
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
    Serial.begin(115200);
    
    // Initialize buttons with internal pullup
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    
    // Initialize OLED
    Wire.begin(5, 4);  // SDA, SCL
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Initialize IR receiver and transmitter
    irrecv.enableIRIn();
    
    drawMenu();
}

void loop() {
    if (irrecv.decode(&results)) {
        handleIRInput();
        irrecv.resume();
    }
    
    handleButtons();
    updateDisplay();
    
    if (isTransmitting) {
        sendPowerCodes();
    }
}

void sendPowerCodes() {
    static unsigned long lastTransmitTime = 0;
    const unsigned long transmitDelay = 100; // Time between codes
    
    if (millis() - lastTransmitTime > transmitDelay) {
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Transmitting...");
        display.print("Code: ");
        display.println(currentCode + 1);
        display.println("Press BACK to stop");
        display.display();
        
        // Send current code
        irrecv.disableIRIn();
        irsend.sendNEC(pgm_read_dword(&POWER_CODES[currentCode]), 32);
        irrecv.enableIRIn();
        
        currentCode = (currentCode + 1) % NUM_CODES;
        lastTransmitTime = millis();
    }
}

void handleButtons() {
    if (millis() - lastDebounceTime < debounceDelay) {
        return;
    }
    
    if (!digitalRead(BTN_UP)) {
        if (!isTransmitting) {
            currentMenuItem = (currentMenuItem - 1 + MENU_ITEMS) % MENU_ITEMS;
            drawMenu();
        }
        lastDebounceTime = millis();
    }
    
    if (!digitalRead(BTN_SELECT)) {
        executeMenuItem();
        lastDebounceTime = millis();
    }
    
    if (!digitalRead(BTN_BACK)) {
        if (isTransmitting) {
            isTransmitting = false;
            currentCode = 0;
            drawMenu();
        } else {
            drawMenu();
        }
        lastDebounceTime = millis();
    }
}

void executeMenuItem() {
    switch(currentMenuItem) {
        case 0: // Record IR
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Ready to record...");
            display.println("Point remote at");
            display.println("receiver and press");
            display.println("any button");
            display.display();
            break;
            
        case 1: // Play Last
            if(lastIRCode != 0) {
                display.clearDisplay();
                display.setCursor(0,0);
                display.println("Transmitting...");
                display.display();
                
                irrecv.disableIRIn();
                irsend.send(irProtocol, lastIRCode, irBits);
                irrecv.enableIRIn();
                
                delay(1000);
            }
            drawMenu();
            break;
            
        case 2: // Show Code
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("IR Code Details:");
            display.print("Code: 0x");
            display.println(lastIRCode, HEX);
            display.print("Protocol: ");
            display.println(irProtocol);
            display.print("Bits: ");
            display.println(irBits);
            display.println("\nPress BACK to exit");
            display.display();
            break;
            
        case 3: // Clear Memory
            lastIRCode = 0;
            irProtocol = 0;
            irBits = 0;
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Memory Cleared!");
            display.display();
            delay(1000);
            drawMenu();
            break;
            
        case 4: // Power Codes
            isTransmitting = true;
            currentCode = 0;
            break;
    }
}

void drawMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    
    for(int i = 0; i < MENU_ITEMS; i++) {
        display.setCursor(0, i * 10);
        if(i == currentMenuItem) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(menuItems[i]);
    }
    
    if(lastIRCode != 0) {
        display.setCursor(0, 45);
        display.print("Last: 0x");
        display.print(lastIRCode, HEX);
    }
    
    display.display();
}

void handleIRInput() {
    lastIRCode = results.value;
    irProtocol = results.decode_type;
    irBits = results.bits;
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("IR Code Received!");
    display.print("Code: 0x");
    display.println(lastIRCode, HEX);
    display.display();
    delay(1000);
    
    drawMenu();
}

void updateDisplay() {
    static unsigned long lastUpdate = 0;
    if(millis() - lastUpdate > 1000) {
        if (!isTransmitting && currentMenuItem != 2) {
            drawMenu();
        }
        lastUpdate = millis();
    }
}
