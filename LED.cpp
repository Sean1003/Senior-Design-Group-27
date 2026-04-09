#include <FastLED.h>

#define LED_PIN     13 
#define NUM_LEDS    64
CRGB leds[NUM_LEDS];

// Snake layout translation
uint16_t getIndex(uint8_t x, uint8_t y) {
    return (y % 2 == 0) ? (y * 8) + x : (y * 8) + (7 - x);
}

// Map UI color codes to FastLED colors
CRGB charToColor(char c) {
    switch (c) {
        case 'r': return CRGB::Red;    case 'g': return CRGB::Green;
        case 'b': return CRGB::Blue;   case 'y': return CRGB::Yellow;
        case 'o': return CRGB::Orange; case 'w': return CRGB::White;
        case 'k': case 'u': return CRGB::Black; 
        default:  return CRGB::Black;
    }
}

void setup() {
    Serial.begin(115200);
    // Important for Web Serial: tells the ESP32 how long to wait for data chunks
    Serial.setTimeout(50); 
    
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(30); // Low brightness for safety while testing on USB
    FastLED.clear(true);
}

void loop() {
    if (Serial.available() > 0) {
        // Read the string sent by the browser until the '!' marker
        String data = Serial.readStringUntil('!');
        
        int x = 0, y = 0;
        String currentCell = "";

        for (int i = 0; i < data.length(); i++) {
            char c = data[i];
            if (c == ',' || c == '\n' || c == '\r') {
                if (currentCell.length() > 0) {
                    char colorLetter = currentCell[currentCell.length() - 1];
                    leds[getIndex(x, y)] = charToColor(colorLetter);
                    if (++x >= 8) { x = 0; y++; }
                    currentCell = "";
                }
            } else if (c != ' ') {
                currentCell += c;
            }
            if (y >= 8) break;
        }
        FastLED.show();
    }
}
