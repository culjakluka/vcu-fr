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


/*
ideja iza ovoga je bila da se nekad podaci salju/primaju
u vecem broju byteova nego sta jedan element data niza moze primit
npr. imamo 2 bytea za definirat power output ali moramo poslat ka 1 byte + 1 byte
*/
uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte);
void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte);

// 0 = bez debug ispisa, 1 = ukljuci debug
#define DEBUG_POWER_LIMITER 0

// Optional: BMS timeout fail-safe 
// 0=off, 1=on (ukljuciti kad bude sve implementirano(sve znajcajke) i kad zelis fail-safe ponasanje)
// Uveli smo BMS timeout fail-safe da, ako BMS prestane slati poruke ili limit ne stigne na vrijeme 
// (npr. pri startu ili prekidu CAN veze), sustav automatski prijede u sigurno stanje tako da postavi 
// powerLimit = 0 i time sprijeci bilo kakav nenamjerni/nesigurni zahtjev za snagom.
// #define ENABLE_BMS_TIMEOUT_FAILSAFE 0
// static const unsigned long BMS_TIMEOUT_MS = 300; // > 10Hz period (100ms)
unsigned long lastBmsRxMs = 0;

int timerTesting;


void setup() {
  Serial.begin(9600);
  SPI.begin();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);

  //ovo je mod za testiranje, inace je setNormalMode() u komunikaciji
  mcp2515.setLoopbackMode();

  Serial.println("Sve ok");

  
  // specifikacije iz zad
  // ID TX i RX poruka
  canTx.can_id = CAN_TX; // 0x200
  canRx.can_id = CAN_RX; // 0x100 (koristi se kao expected ID)

 //duljina data unutar okvira sta se salje, u bajtovima (svako polje je 1 bajt)
  canTx.can_dlc = 6;
  canRx.can_dlc = 3;

  timerReceiver = 0;
  timerTransmitter = 0;

  // init vrijednosti (da ne saljem smece)
  vcuState = IDLE;
  bmsState = IDLE;
  plausibleRequest = 0;
  mapMode = ECO;
  powerRequest = 0;
  powerLimit = 0;

  lastBmsRxMs = 0;

  pinMode(ledMappingMode, OUTPUT);
  pinMode(ledPowerRequest, OUTPUT);

  pinMode(btnMappingMode, INPUT_PULLUP);
  pinMode(btnVCUState, INPUT_PULLUP);

  updateLED(mapMode, ledMappingMode);

  randomSeed(analogRead(3));

  timerTesting = 0;
}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  simulateBmsNode();

  // FEATURE 1: CAN_RX (10 Hz): citaj BMS poruku 0x100

  if (millis() - timerReceiver > 100) {
    timerReceiver = millis();

    if (mcp2515.readMessage(&canRx) == MCP2515::ERROR_OK) {
      // filtriraj: prihvati samo CAN_RX (0x100)
      if ((canRx.can_id == CAN_RX) & (canRx.can_dlc == 3)) {
        bmsState = (NodeState)canRx.data[0];
        powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
        //lastBmsRxMs = millis(); // <-- biljezi kada je zadnji put dosao svjezi limit
      }
    }
  }

  updateMapping(btnMappingMode, mapMode, ledMappingMode);

  if(VcuBtnDebounce(btnVCUState)){
    vcuState = VcuChangeState(bmsState, checkPlausible(pot1, pot2), vcuState);
  }

  VcuManageState(vcuState, pot1, pot2, plausibleRequest, powerRequest, mapMode);

  if(millis() - timerTesting > 1000){
    timerTesting = millis();
    Serial.print("Plausible difference: ");
    Serial.println(checkPlausible(pot1, pot2));
    Serial.print("plausible request: ");
    Serial.println(getPlausibleReq(pot1, pot2));
    Serial.print("vcu node state: ");
    Serial.println(vcuState);
    Serial.print("bms node state: ");
    Serial.println(bmsState);
    Serial.print("power request: ");
    Serial.println(powerRequest);
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


void simulateBmsNode(){

  struct can_frame bmsMsg;

  bmsMsg.can_id = CAN_RX;
  bmsMsg.can_dlc = 3;

  static int timerSendBms = 0;
  uint16_t powerLimit = 30000;
  NodeState bmsState = READY;
  static int timerChangeBmsState = 0;
  int changePeriod = 3000;
  int chooseState = 1;

  if(millis() - timerChangeBmsState > changePeriod){

    changePeriod = random(1000, 7000);
    timerChangeBmsState = millis();
    chooseState = random(0, 3);

    switch(chooseState){
      case 0:
        bmsState = IDLE;
        break;
      case 1:
        bmsState = READY;
        break;
      default:
        bmsState = ERROR;
        break;
    }
  }

  if(millis() - timerSendBms > 100){

    timerSendBms = millis();

    powerLimit = random(30000, 65500);
    bmsMsg.data[0] = bmsState;
    encryptBytes(powerLimit, bmsMsg.data[1], bmsMsg.data[2]);

    // Serial.print("sent power limit: ");
    // Serial.print(powerLimit);
    // Serial.print(" sent bms state: ");
    // Serial.println(chooseState);

    auto err = mcp2515.sendMessage(&bmsMsg);
  }
}


