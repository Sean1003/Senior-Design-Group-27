#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>

#define SDA_PIN 3
#define SCL_PIN 46
#define LED_PIN 21
#define NUM_LEDS 64
#define MOTOR_ADDRESS 0x20
#define SOL_BUS1_ADDRESS  0x21
#define SOL_BUS2_ADDRESS  0x22
#define SOL_BUS3_ADDRESS  0x23
#define SOL_BUS4_ADDRESS  0x24
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define LEVEL_INCREMENT 1200

// MCP23017 registers
#define IODIRA 0x00
#define IODIRB 0x01
#define GPIOA  0x12
#define GPIOB  0x13

//Arrays being used for storing the input data
int levels_0[64];
char colors_0[64];

//Array that stores the LED indexes that correspond to the grid
uint8_t led_index[64];

int levels_0_index = 0;
int colors_0_index = 0;

CRGB leds[NUM_LEDS];
int I2C_INIT = 1;


void writeRegister(uint8_t reg, uint8_t value, uint8_t address);
void initialize_pinExpanders();
void Adjust_Height(int solenoidNumber, int levels);
void Turn_On_Solenoid(int solenoidIndex);
void Turn_Off_Solenoid(int solenoidIndex);
void Turn_On_Motor(int solenoidIndex);
void Turn_On_Motor_Reverse(int solenoidIndex);
void Turn_Off_Motor(int solenoidIndex);
void initialize_leds();
int led_correction(int index);
uint16_t getIndex(uint8_t x, uint8_t y);
CRGB charToColor(char c);

void setup() {
  Serial.begin(115200);
  Serial.println("BOOT");
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100kHz (safe default)
  Serial.println("ESP Intialized\n\r");

  //Start I2C transmission
  uint8_t error;

  //Test if the I2C devices are found 
  while(I2C_INIT) {
    Serial.print("Waiting for I2C connection...\n\r");

    Wire.beginTransmission(MOTOR_ADDRESS);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.println("Motor Device ACKed (found)\n\r");
      I2C_INIT = 0;
    }   else {
      Serial.print("No Motor ACK, error code: \n\r");
    }

    Wire.beginTransmission(SOL_BUS2_ADDRESS);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.println("SOL2 Device ACKed (found)\n\r");
      I2C_INIT = 0;
    } else {
      Serial.print("No SOL2 ACK, error code: \n\r");
    }
    delay(1000);
  }
  Serial.print("I2C Device Found\n\r");

   while (!Serial) {
    delay(10);
  }

  //Initialize the LED array
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255); // Global brightness at max; we control intensity per-LED
  FastLED.clear(true);

  initialize_pinExpanders();
  initialize_leds();
}

String rawData = "";
int loop_count = 0;
bool dataReceived = false;
bool startReceiving = false;
int dataIndex = 0;
//LOOP BEGINS
void loop() {

  for(int i=0; i<64; i++) {
    leds[led_correction(i)] = CRGB(255,0,255);
    FastLED.show();
  }

  Turn_On_Motor(8);
  delay(5000);
  Turn_Off_Motor(8);
  Turn_On_Motor(40);
  delay(5000);
  Turn_Off_Motor(40);
  

  delay(3000);
  /*

  for(int i=0; i<64; i++) {
    Adjust_Height(i, 3);
    Turn_On_Motor_Reverse(i);
    delay(200);
    Turn_Off_Motor(i);
  }
  delay(1000000);
  */
  /*
  if(!dataReceived) {
  if(Serial.available()) {
    String data = Serial.readStringUntil('\n');
    for(int i=0; i<data.length(); i++) {
      if(startReceiving && data[i] != 'E') {
        rawData += data[i];
      }
      if(data[i] == 'S'){
        startReceiving = true;
        leds[1] = CRGB(255,255,255);
        FastLED.show();
      }
      if(data[i] == 'E') {
        dataReceived = true;
        leds[2] = CRGB(255,0,0);
        FastLED.show();
      }
      
    }
  }
  }
  else {
    for(int i=0; i<184; i+=23) {
      levels_0[levels_0_index++] = rawData[i];
      colors_0[colors_0_index++] = rawData[i+1];

      levels_0[levels_0_index++] = rawData[i+3];
      colors_0[colors_0_index++] = rawData[i+4];

      levels_0[levels_0_index++] = rawData[i+6];
      colors_0[colors_0_index++] = rawData[i+7];

      levels_0[levels_0_index++] = rawData[i+9];
      colors_0[colors_0_index++] = rawData[i+10];

      levels_0[levels_0_index++] = rawData[i+12];
      colors_0[colors_0_index++] = rawData[i+13];

      levels_0[levels_0_index++] = rawData[i+15];
      colors_0[colors_0_index++] = rawData[i+16];

      levels_0[levels_0_index++] = rawData[i+18];
      colors_0[colors_0_index++] = rawData[i+19];

      levels_0[levels_0_index++] = rawData[i+21];
      colors_0[colors_0_index++] = rawData[i+22];
    }


    for(int i=0; i<64; i++) {
      leds[led_correction(i)] = charToColor(colors_0[i]);
      FastLED.show();
    }
      
    delay(100000);
  }
    */
}

//LOOP ENDS


void writeRegister(uint8_t reg, uint8_t value, uint8_t address) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initialize_pinExpanders() {
  uint8_t address = 0x20;
  for(int i=0; i<5; i++) {
    //Serial.print(address);
    writeRegister(IODIRA, 0b00000000, address);
    writeRegister(IODIRB, 0b00000000, address);
    writeRegister(GPIOA, 0b11111111, address);
    writeRegister(GPIOB, 0b11111111, address);
    address += 1;
  }
}

void Adjust_Height(int solenoidIndex, int levels) {
  if(levels == 0) {
    return;
  }
  Turn_On_Solenoid(solenoidIndex);
  Turn_On_Motor(solenoidIndex);

  if(levels == 1) {
    delay(LEVEL_INCREMENT);
  }
  else if(levels == 2) {
    delay(LEVEL_INCREMENT * 2);
  }
  else if(levels == 3) {
    delay(LEVEL_INCREMENT * 3);
  }

  initialize_pinExpanders();
  writeRegister(GPIOB, 0b11111111, MOTOR_ADDRESS);
  writeRegister(GPIOA, 0b11111111, MOTOR_ADDRESS);
}

void Turn_On_Motor(int solenoidIndex) {
  if(solenoidIndex >=0 && solenoidIndex <=7) {
    writeRegister(GPIOA, 0b11111011, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=8 && solenoidIndex <=15) {
    writeRegister(GPIOA, 0b11111110, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=16 && solenoidIndex <=23) {
    writeRegister(GPIOA, 0b10111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=24 && solenoidIndex <=31) {
    writeRegister(GPIOA, 0b11101111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=32 && solenoidIndex <=39) {
    writeRegister(GPIOB, 0b11111011, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=40 && solenoidIndex <=47) {
    writeRegister(GPIOB, 0b11111110, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=48 && solenoidIndex <=55) {
    writeRegister(GPIOB, 0b11011111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=56 && solenoidIndex <=63) {
    writeRegister(GPIOB, 0b01111111, MOTOR_ADDRESS);
  }
}

void Turn_On_Motor_Reverse(int solenoidIndex) {
  if(solenoidIndex >=0 && solenoidIndex <=7) {
    writeRegister(GPIOA, 0b11110111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=8 && solenoidIndex <=15) {
    writeRegister(GPIOA, 0b11111101, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=16 && solenoidIndex <=23) {
    writeRegister(GPIOA, 0b01111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=24 && solenoidIndex <=31) {
    writeRegister(GPIOA, 0b11011111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=32 && solenoidIndex <=39) {
    writeRegister(GPIOB, 0b11110111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=40 && solenoidIndex <=47) {
    writeRegister(GPIOB, 0b11111101, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=48 && solenoidIndex <=55) {
    writeRegister(GPIOB, 0b11101111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=56 && solenoidIndex <=63) {
    writeRegister(GPIOB, 0b10111111, MOTOR_ADDRESS);
  }
}

void Turn_Off_Motor(int solenoidIndex) {
  if(solenoidIndex >=0 && solenoidIndex <=7) {
    writeRegister(GPIOA, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=8 && solenoidIndex <=15) {
    writeRegister(GPIOA, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=16 && solenoidIndex <=23) {
    writeRegister(GPIOA, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=24 && solenoidIndex <=31) {
    writeRegister(GPIOA, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=32 && solenoidIndex <=39) {
    writeRegister(GPIOB, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=40 && solenoidIndex <=47) {
    writeRegister(GPIOB, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=48 && solenoidIndex <=55) {
    writeRegister(GPIOB, 0b11111111, MOTOR_ADDRESS);
  }
  else if(solenoidIndex >=56 && solenoidIndex <=63) {
    writeRegister(GPIOB, 0b11111111, MOTOR_ADDRESS);
  }
}

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

void Turn_On_Solenoid(int solenoidIndex) {
  
  uint8_t device;
  int solenoidNumber = solenoidIndex % 16;
  Serial.println(solenoidNumber);

  if(solenoidIndex >=0 && solenoidIndex <=15) {
    device = SOL_BUS1_ADDRESS;
  }
  else if(solenoidIndex >=16 && solenoidIndex <=31) {
    device = SOL_BUS2_ADDRESS;
  }
  else if(solenoidIndex >=32 && solenoidIndex <=47) {
    device = SOL_BUS3_ADDRESS;
  }
  else if(solenoidIndex >=48 && solenoidIndex <=63) {
    device = SOL_BUS4_ADDRESS;
  }

  
  switch(solenoidNumber) {
    case 0:
      writeRegister(GPIOB, 0b11111110, device);
      break;
    case 1:
      writeRegister(GPIOB, 0b11111101, device);
      break;
    case 2:
      writeRegister(GPIOB, 0b11111011, device);
      break;
    case 3:
      writeRegister(GPIOB, 0b11110111, device);
      break;
    case 4:
      writeRegister(GPIOB, 0b11101111, device);
      break;
    case 5:
      writeRegister(GPIOB, 0b11011111, device);
      break;
    case 6:
      writeRegister(GPIOB, 0b10111111, device);
      break;
    case 7:
      writeRegister(GPIOB, 0b01111111, device);
      break;
    case 8:
      writeRegister(GPIOA, 0b11111110, device);
      break;
    case 9:
      writeRegister(GPIOA, 0b11111101, device);
      break;
    case 10:
      writeRegister(GPIOA, 0b11111011, device);
      break;
    case 11:
      writeRegister(GPIOA, 0b11110111, device);
      break;
    case 12:
      writeRegister(GPIOA, 0b11101111, device);
      break;
    case 13:
      writeRegister(GPIOA, 0b11011111, device);
      break;
    case 14:
      writeRegister(GPIOA, 0b10111111, device);
      break;
    case 15:
      writeRegister(GPIOA, 0b01111111, device);
      break;
  }

}

void Turn_Off_Solenoid(int solenoidIndex) {
  
  uint8_t device;
  int solenoidNumber = solenoidIndex % 16;
  Serial.println(solenoidNumber);

  if(solenoidIndex >=0 && solenoidIndex <=15) {
    device = SOL_BUS1_ADDRESS;
  }
  else if(solenoidIndex >=16 && solenoidIndex <=31) {
    device = SOL_BUS2_ADDRESS;
  }
  else if(solenoidIndex >=32 && solenoidIndex <=47) {
    device = SOL_BUS3_ADDRESS;
  }
  else if(solenoidIndex >=48 && solenoidIndex <=63) {
    device = SOL_BUS4_ADDRESS;
  }

  switch(solenoidNumber) {
    case 0:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 1:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 2:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 3:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 4:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 5:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 6:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 7:
      writeRegister(GPIOB, 0b11111111, device);
      break;
    case 8:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 9:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 10:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 11:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 12:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 13:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 14:
      writeRegister(GPIOA, 0b11111111, device);
      break;
    case 15:
      writeRegister(GPIOA, 0b11111111, device);
      break;
  }
}

void initialize_leds() {
  for(int i=0; i<64; i++) {
    if(i<8) {
      led_index[i] = i+56;
    }
    else if(i<16) {
      switch(i) {
        case 15: led_index[i] = 48; break;
        case 14: led_index[i] = 49; break;
        case 13: led_index[i] = 50; break;
        case 12: led_index[i] = 51; break;
        case 11: led_index[i] = 52; break;
        case 10: led_index[i] = 53; break;
        case 9:  led_index[i] = 54; break;
        case 8:  led_index[i] = 55; break;
      }
    }
    else if(i<24) {
        led_index[i] = i + 24;
    }
    else if(i<32) {
      switch(i) {
        case 31: led_index[i] = 32; break;
        case 30: led_index[i] = 33; break;
        case 29: led_index[i] = 34; break;
        case 28: led_index[i] = 35; break;
        case 27: led_index[i] = 36; break;
        case 26: led_index[i] = 37; break;
        case 25:  led_index[i] = 38; break;
        case 24:  led_index[i] = 39; break;
      }
    }
    else if(i<40) {
      led_index[i] = i - 8;
    }
    else if(i<48) {
      switch(i){
      case 47: led_index[i] = 16; break;
      case 46: led_index[i] =17; break;
      case 45: led_index[i] = 18; break;
      case 44: led_index[i] = 19; break;
      case 43: led_index[i] = 20; break;
      case 42: led_index[i] = 21; break;
      case 41: led_index[i] = 22; break;
      case 40: led_index[i] = 23; break;
      }
    }
    else if(i<56) {
      led_index[i] = i - 40;
    }
    else if(i<64) {
      switch(i){
      case 63: led_index[i] = 0; break;
      case 62: led_index[i] =1; break;
      case 61: led_index[i] = 2; break;
      case 60: led_index[i] = 3; break;
      case 59: led_index[i] = 4; break;
      case 58: led_index[i] = 5; break;
      case 57: led_index[i] = 6; break;
      case 56: led_index[i] = 7; break;
      }
    }
  }
}

int led_correction(int index) {
  return led_index[index];
}