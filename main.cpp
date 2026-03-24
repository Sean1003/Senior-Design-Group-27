#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>

#define SDA_PIN 3
#define SCL_PIN 46
#define LED_PIN 21
#define NUM_LEDS 2
#define MOTOR_ADDRESS 0x20
#define SOL1_ADDRESS  0x21
#define SOL2_ADDRESS  0x22
#define SOL3_ADDRESS  0x23
#define SOL4_ADDRESS  0x24

// MCP23017 registers
#define IODIRA 0x00
#define IODIRB 0x01
#define GPIOA  0x12
#define GPIOB  0x13

CRGB leds[NUM_LEDS];
int I2C_INIT = 1;

void writeRegister(uint8_t reg, uint8_t value, uint8_t address);
void initialize_pinExpanders();
int Turn_On_Solenoid(int solenoidNumber, int deviceNumber);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100kHz (safe default)
  Serial.println("ESP Intialized\n\r");

  //Start I2C transmission
  Wire.begin(SDA_PIN, SCL_PIN);
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

    Wire.beginTransmission(SOL1_ADDRESS);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.println("SOL1 Device ACKed (found)\n\r");
      I2C_INIT = 0;
    } else {
      Serial.print("No SOL1 ACK, error code: \n\r");
    }
    delay(1000);
  }

  Serial.print("I2C Device Found\n\r");
  initialize_pinExpanders();
}

void loop() {
  
  Serial.println("Entering Loop...\n\r");

  // LED 0 = red, LED 1 = green
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  FastLED.show();
  delay(1000);

  // swap colors
  leds[0] = CRGB::Blue;
  leds[1] = CRGB::White;
  FastLED.show();
  delay(1000);
}

//FUNCTIONS


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

int Turn_On_Solenoid(int solenoidNumber, int deviceNumber) {
  
  uint8_t device;
  uint8_t GP_Dir;
  return 0;

  switch(deviceNumber) {
    case 1:
      device = SOL1_ADDRESS; 
      break;
    case 2:
      device = SOL2_ADDRESS;
      break;
    case 3:
      device = SOL3_ADDRESS;
      break;
    case 4:
      device = SOL4_ADDRESS;
      break;
  }
  
  switch(solenoidNumber) {
    case 0:
      writeRegister(GPIOB, 0b01111111, device);
      break;
    case 1:
      writeRegister(GPIOB, 0b10111111, device);
      break;
    case 2:
      writeRegister(GPIOB, 0b11011111, device);
      break;
    case 3:
      writeRegister(GPIOB, 0b11101111, device);
      break;
    case 4:
      writeRegister(GPIOB, 0b11110111, device);
      break;
    case 5:
      writeRegister(GPIOB, 0b11111011, device);
      break;
    case 6:
      writeRegister(GPIOB, 0b11111101, device);
      break;
    case 7:
      writeRegister(GPIOB, 0b11111110, device);
      break;
    case 8:
      writeRegister(GPIOA, 0b11111110, device);
      break;
    case 9:
      writeRegister(GPIOA, 0b11111101, device);
      break;
    case 10:
      break;
    case 11:
      break;
    case 12:
      break;
    case 13:
      break;
    case 14:
      break;
    case 15:
      break;
  }

}