#pragma once // kako bi se sprijecio dupli include
#include <SPI.h>
#include <mcp2515.h>
#include <Arduino.h>

#define CAN_ID_MASK 0x7FF


//isti su stateovi za VCU i BMS node
enum NodeState: uint8_t {
  IDLE,
  READY,
  ERROR
};

enum Mode: uint8_t {
  ECO,
  NORMAL,
  SPORT
};

enum ID {
  CAN_RX = 0x100,
  CAN_TX = 0x200
};


MCP2515 mcp2515(10); // CS pin 10

struct can_frame canTx;
struct can_frame canRx;

unsigned long timerReceiver;
unsigned long timerTransmitter;
bool plausibilityState;

// receiver podatci
NodeState bmsState;
uint16_t powerLimit;

// transmitter podatci
NodeState vcuState;
uint16_t plausibleRequest;
Mode mapMode;
uint16_t powerRequest;

// led pinovi
int ledMappingMode = 3;
int ledPowerRequest = 5;

// btn pinovi
int btnMappingMode = 7;
int btnVCUState = 8;

// pot vrijednosti
int pot1;
int pot2;

unsigned long timerTesting;

uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte);
void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte);

void setup() {
  Serial.begin(9600);
  SPI.begin();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);

  //ovo je mod za testiranje, inace je setNormalMode() u komunikaciji
  mcp2515.setNormalMode();

  Serial.println("Sve ok");

  timerReceiver = 0;
  timerTransmitter = 0;

  vcuState = IDLE;
  bmsState = IDLE;
  plausibleRequest = 0;
  mapMode = ECO;
  powerRequest = 0;
  powerLimit = 0;

  pinMode(ledMappingMode, OUTPUT);
  pinMode(ledPowerRequest, OUTPUT);

  pinMode(btnMappingMode, INPUT_PULLUP);
  pinMode(btnVCUState, INPUT_PULLUP);


  timerTesting = 0;
}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  //100ms jer je frekvencija 10Hz
  // if (millis() - timerReceiver > 100) {
  //   timerReceiver = millis();
  auto msg = mcp2515.readMessage(&canRx);
  if(msg == MCP2515::ERROR_OK) {
    Serial.println("uslo");
    // filtriraj: prihvati samo CAN_RX (0x100)
    if (((canRx.can_id) == CAN_RX) & (canRx.can_dlc == 3)) {
      bmsState = (NodeState)canRx.data[0];
      powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
    }
  }
  // }

  if(millis() - timerTesting > 1000){
    timerTesting = millis();
    Serial.print("BMS state:");
    Serial.println(bmsState);
    Serial.print("power limit: ");
    Serial.println(powerLimit);
  }

}


uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte){
  return (uint16_t)(secondByte << 8) | firstByte;
}

void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte){
  firstByte = value & 255;
  secondByte = (value >> 8) & 255;
  return ;
}




