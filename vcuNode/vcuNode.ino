#include "node.h"



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

uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte);
void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte);

void setup() {
  Serial.begin(9600);
  SPI.begin();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);

  //ovo je mod za testiranje, inace je setNormalMode() u komunikaciji
  mcp2515.setNormalMode();

  Serial.println("Sve ok");

  
  // ID TX i RX poruka
  canTx.can_id = CAN_TX;
  canRx.can_id = CAN_RX;

  //duljina data unutar okvira sta se salje, u bajtovima (svako polje je 1 bajt)
  canTx.can_dlc = 6;
  canRx.can_dlc = 3;

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

  updateLED(mapMode, ledMappingMode);
}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  //100ms jer je frekvencija 10Hz
  if (millis() - timerReceiver > 100) {
    timerReceiver = millis();

    if (mcp2515.readMessage(&canRx) == MCP2515::ERROR_OK) {
      // filtriraj: prihvati samo CAN_RX (0x100)
      if ((canRx.can_id == CAN_RX) & (canRx.can_dlc == 3)) {
        bmsState = (NodeState)canRx.data[0];
        powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
      }
    }
  }

  updateMapping(btnMappingMode, mapMode, ledMappingMode);

  if(VcuBtnDebounce(btnVCUState)){
    vcuState = VcuChangeState(bmsState, checkPlausible(pot1, pot2), vcuState);
  }

  VcuManageState(vcuState, pot1, pot2, plausibleRequest, powerRequest, mapMode);

  analogWrite(ledPowerRequest, powerW_to_pwm(powerRequest));

  if(millis() - timerTransmitter > 50){
    timerTransmitter = millis();
    canTx.data[0] = vcuState;
    encryptBytes(plausibleRequest, canTx.data[1], canTx.data[2]);
    canTx.data[3] = mapMode;
    encryptBytes(powerRequest, canTx.data[4], canTx.data[5]);
    auto msg = mcp2515.sendMessage(&canTx);
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




