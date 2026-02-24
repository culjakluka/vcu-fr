#include "node.h"

MCP2515 mcp2515(10); // CS pin 10

struct can_frame canTx;
struct can_frame canRx;

int timerReceiver;
int timerTransmitter;
bool plausibilityState;

//receiver podatci
NodeState bmsState;
uint16_t powerLimit;

//transmitter podatci
NodeState vcuState;
uint16_t plausibleRequest;
Mode mapMode;
uint16_t powerRequest;

//led pinovi
int ledMappingMode = 3;
int ledPowerRequest = 5;

//btn pinovi
int btnMappingMode = 7;
int btnVCUState = 8;

//pot vrijednosti
int pot1;
int pot2;




/*
ideja iza ovoga je bila da se nekad podaci salju/primaju 
u vecem broju byteova nego sta jedan element data niza moze primit
npr. imamo 2 bytea za definirat power output ali moramo poslat ka 1 byte + 1 byte
*/
uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte);
void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte);





void setup() {
  // basic setup
  Serial.begin(9600);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  //ovo je mod za testiranje, inace je setNormalMode() u komunikaciji
  mcp2515.setLoopbackMode();
  Serial.println("sve ok");

  // specifikacije iz zad
  //ID TX i RX poruka
  canTx.can_id = CAN_TX;
  canRx.can_id = CAN_RX;

  //duljina data unutar okvira sta se salje, u bajtovima (svako polje je 1 bajt)
  canTx.can_dlc = 6;
  canRx.can_dlc = 3;

  timerReceiver = 0;
  timerTransmitter = 0;

  pinMode(ledMappingMode, OUTPUT);
  pinMode(ledPowerRequest, OUTPUT);

  pinMode(btnMappingMode, INPUT_PULLUP);
  pinMode(btnVCUState, INPUT_PULLUP);

}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  pedalValue(pot1, pot2, plausibleRequest, plausibilityState);

  //frekvencija RX je 10Hz
  if(millis() - timerReceiver > 100){

    timerReceiver = millis();

    if (mcp2515.readMessage(&canRx) == MCP2515::ERROR_OK) {
        //poruka primljena
        bmsState = canRx.data[0];
        powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
    }
    else{
      //poruka nije primljena, moze se racunat ka ERROR state od BMS-a
    }
  }

  //frekvencija TX je 20Hz
  if(millis() - timerTransmitter > 50){

    timerTransmitter = millis();

    canTx.data[0] = vcuState;


    encryptBytes(plausibleRequest, canTx.data[1], canTx.data[2]);
    canTx.data[3] = mapMode;
    encryptBytes(powerRequest, canTx.data[4], canTx.data[5]);
    auto err = mcp2515.sendMessage(&canTx);

    if(err == MCP2515::ERROR_OK){
      //poruka poslana
    }
    else{
      //poruka nije poslana
    }
  }

}

//ove funkcije nisu testirane jos
uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte){
  /*
  ako je 
  firstbyte = 1001101
  secondbyte = 00101001
  onda drugi bajt shifta za 8 bitova ulijevo --> 0010100100000000
  i rezultat spoji sa prvim bajtom --> 001010011001101      
  */
  return ((uint16_t)secondByte << 8) | firstByte;
}

void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte){
  firstByte = value & 255;
  secondByte = (value >> 8) & 255;
  return ;
}








