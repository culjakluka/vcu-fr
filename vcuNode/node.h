#include <SPI.h>
#include <mcp2515.h>
#include <Arduino.h>

//deklaracije funkcija i drugih stvari

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


//deklaracije za stateMachine feature








//deklaracije za mappingMode feature

void initMapping(int ledPin, int btnPin);
void updateMapping();
void updateLED();
uint16_t applyMapping(uint16_t plausibleRequest);



//deklaracije za plausibility feature








//deklaracije za powerLimiter feature



















