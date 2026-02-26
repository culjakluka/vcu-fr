#pragma once // kako bi se sprijecio dupli include
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

bool VcuBtnDebounce(int btn);
void VcuChangeState(NodeState bms, bool plausible, NodeState &vcu);
void VcuManageState(NodeState vcu, uint16_t &powerReq, uint16_t plausibileReq);



//deklaracije za mappingMode feature

void initMapping(int ledPin, int btnPin);
void updateMapping();
void updateLED();
uint16_t applyMapping(uint16_t plausibleRequest);



//deklaracije za plausibility feature
void pedalValue(int pot1, int pot2, int plausibleRequest, bool isPlausible);







//deklaracije za powerLimiter feature
uint16_t pct10_to_powerW(uint16_t pct10);
uint16_t applyPowerLimit(uint16_t requestedPowerW, uint16_t powerLimitW);
uint8_t powerW_to_pwm(uint16_t powerW);

















