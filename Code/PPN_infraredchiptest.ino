/*
  IR Signal Receiver & Repeater for Arduino Nano

  This sketch decodes incoming infrared (IR) signals on pin D2 and stores
  the last received signal. When a button on pin D6 is pressed, it
  re-transmits (plays back) the stored signal from pin D3.

  This version is corrected to work with IRremote library version 2.6.0.

  Circuit:
  - IR Receiver (e.g., TSOP1738):
    - VCC to Arduino 5V
    - GND to Arduino GND
    - Signal/Data to Arduino Pin 2
  - IR Transmitter (IR LED):
    - Anode (long lead) to a 220-ohm resistor, then to Arduino 5V.
    - Cathode (short lead) to the Collector of an NPN Transistor (e.g., 2N2222).
    - The transistor's Emitter to Arduino GND.
    - The transistor's Base to Arduino Pin 3 (via a 1k-ohm resistor).
  - Pushbutton:
    - One leg to Arduino Pin 6.
    - The other leg to Arduino GND.

  Required Library:
  - IRremote by shiriff, z3t0 (Version 2.6.0)
    -> https://github.com/Arduino-IRremote/Arduino-IRremote
*/

#include <IRremote.h>

// --- Pin Definitions ---
const int RECV_PIN = 2;    // Input from IR receiver
const int BUTTON_PIN = 6;  // Input from playback button
// The IRremote library uses Pin 3 for the sender by default.

// --- IR Objects ---
IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results; // Structure to hold received data

// --- Storage for the last signal ---
// We need to store all the info to be able to re-send it.
struct {
  decode_type_t protocol;
  unsigned long value;
  int bits;
  bool captured = false; // Flag to check if we have a signal stored
} lastSignal;


void setup() {
  Serial.begin(9600);
  Serial.println("IR Receiver/Repeater Initialized.");

  // Configure the button pin with an internal pull-up resistor.
  // The pin will be HIGH by default and LOW when the button is pressed.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Start the IR receiver.
  irrecv.enableIRIn();
}

void loop() {
  // 1. Check for and decode an incoming IR signal.
  if (irrecv.decode(&results)) {
    Serial.println("--- Signal Captured! ---");
    Serial.print("Protocol: ");
    Serial.println(getProtocolString(results.decode_type));
    Serial.print("Value (HEX): 0x");
    Serial.println(results.value, HEX);

    // Store the decoded signal's information for playback.
    // Note: The library uses FFFFFFFF for repeat codes, we store them as well.
    lastSignal.protocol = results.decode_type;
    lastSignal.value = results.value;
    lastSignal.bits = results.bits;
    lastSignal.captured = true; // Mark that we have a signal to play back

    // Resume listening for the next signal.
    irrecv.resume();
  }

  // 2. Check if the playback button is pressed.
  // digitalRead is LOW when the button is pressed because of INPUT_PULLUP.
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (lastSignal.captured) {
      Serial.println(">>> Playback button pressed. Sending last signal...");

      // The IRsend object will automatically use pin 3.
      // Use a switch to call the correct sending function based on the stored protocol.
      switch(lastSignal.protocol) {
        case NEC: irsend.sendNEC(lastSignal.value, lastSignal.bits); break;
        case SONY: irsend.sendSony(lastSignal.value, lastSignal.bits); break;
        case RC5: irsend.sendRC5(lastSignal.value, lastSignal.bits); break;
        case RC6: irsend.sendRC6(lastSignal.value, lastSignal.bits); break;
        case SAMSUNG: irsend.sendSAMSUNG(lastSignal.value, lastSignal.bits); break;
        case LG: irsend.sendLG(lastSignal.value, lastSignal.bits); break;
        case PANASONIC: irsend.sendPanasonic(lastSignal.value, lastSignal.bits); break;
        // Add other protocols as needed
        default: Serial.println("Cannot play back UNKNOWN protocol.");
      }
      
      // *** FIX: Re-enable the receiver after sending ***
      // Sending an IR signal often disables the receiver's timer.
      // This line re-initializes it so it's ready to capture the next signal.
      irrecv.enableIRIn();

      // A small delay to debounce the button and prevent rapid-fire sending.
      delay(250);

    } else {
      Serial.println("Playback button pressed, but no signal captured yet.");
      delay(250); // Delay to prevent spamming the serial monitor
    }
  }
}

// Helper function to convert protocol enum to a human-readable string.
const char* getProtocolString(decode_type_t protocol) {
  switch (protocol) {
    case UNKNOWN:      return "UNKNOWN";
    case NEC:          return "NEC";
    case SONY:         return "SONY";
    case RC5:          return "RC5";
    case RC6:          return "RC6";
    case DISH:         return "DISH";
    case SHARP:        return "SHARP";
    case JVC:          return "JVC";
    case SANYO:        return "SANYO";
    case MITSUBISHI:   return "MITSUBISHI";
    case SAMSUNG:      return "SAMSUNG";
    case LG:           return "LG";
    case WHYNTER:      return "WHYNTER";
    case PANASONIC:    return "PANASONIC";
    case DENON:        return "DENON";
    default:           return "OTHER";
  }
}
