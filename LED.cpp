#include <FastLED.h>

// --- CONFIGURATION ---
#define LED_PIN     13        // Data pin for WS2812B
#define NUM_LEDS    64        // 8x8 Grid
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// --- HELPER FUNCTIONS ---

// Translates 8x8 coordinates to a Serpentine (Snake) path
uint16_t getIndex(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {
        return (y * 8) + x;           // Even rows: Left to Right
    } else {
        return (y * 8) + (7 - x);     // Odd rows: Right to Left
    }
}

// Maps UI color characters to FastLED CRGB colors
CRGB charToColor(char c) {
    switch (c) {
        case 'r': return CRGB::Red;
        case 'g': return CRGB::Green;
        case 'b': return CRGB::Blue;
        case 'y': return CRGB::Yellow;
        case 'o': return CRGB::Orange;
        case 'w': return CRGB::White;
        case 'k': return CRGB::Black; // Black
        case 'u': return CRGB::Black; // Unassigned
        default:  return CRGB::Black;
    }
}

void setup() {
    // High-speed serial for real-time Web Serial API
    Serial.begin(115200);
    
    // Short timeout ensures the ESP32 processes "Real-time" color drags quickly
    Serial.setTimeout(20); 

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255); // Global brightness at max; we control intensity per-LED
    FastLED.clear(true);
}

void loop() {
    if (Serial.available() > 0) {
        // Read until the '!' end-marker sent by the JavaScript UI
        String data = Serial.readStringUntil('!');
        
        int x = 0;
        int y = 0;
        String currentCell = "";

        // Parse the CSV-style string: "0g,2r,3b..."
        for (int i = 0; i < data.length(); i++) {
            char c = data[i];

            if (c == ',' || c == '\n' || c == '\r') {
                if (currentCell.length() >= 2) {
                    // 1. Level (Intensity): First character (0, 1, 2, or 3)
                    int level = currentCell[0] - '0'; 
                    
                    // 2. Color: Second character
                    char colorLetter = currentCell[1]; 
                    
                    // 3. Map Level to Brightness (0=Dim, 3=Full)
                    uint8_t brightnessMap[] = {30, 80, 160, 255}; 
                    uint8_t targetBrightness = brightnessMap[level];

                    int ledIndex = getIndex(x, y);
                    CRGB color = charToColor(colorLetter);
                    
                    // Apply the color and scale its brightness based on the Level
                    leds[ledIndex] = color.nscale8(targetBrightness);
                    
                    if (++x >= 8) { 
                        x = 0; 
                        y++; 
                    }
                }
                currentCell = ""; 
            } else if (c != ' ') {
                currentCell += c;
            }
            
            if (y >= 8) break; // Stop if grid is full
        }
        
        FastLED.show();
    }
}
