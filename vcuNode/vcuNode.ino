#include <SPI.h>
#include <mcp2515.h>
#include "node.h"

MCP2515 mcp2515(10); // CS pin 10

struct can_frame canTx;
struct can_frame canRx;

int timerReceiver;
int timerTransmitter;

//receiver podatci
uint8_t bmsState;
uint16_t powerLimit;

//transmitter podatci
uint8_t vcuState;
uint16_t plausibleRequest;
uint8_t mapMode;
uint16_t powerRequest;

//led pinovi
int led1 = 3;
int led2 = 5;

//btn pinovi
int btn1 = 7;
int btn2 = 8;

//pot
int pot1;
int pot2;


/*
ideja iza ovoga je bila da se nekad podaci salju/primaju 
u vecem broju byteova nego sta jedan element data niza moze primit
npr. imamo 2 bytea za definirat power output ali moramo poslat ka 1 byte + 1 byte
*/
//uint16_t decodeBytes(uint8_t firstByte, uint8_t secondByte);
//void encryptBytes(uint16_t value, uint8_t &firstByte, uint8_t &secondByte);

void setup() {
  // basic setup
  Serial.begin(9600);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  //ovo je mod za testiranje, inace je setNormalMode()
  mcp2515.setLoopbackMode();
  Serial.println("sve ok");

  // specifikacije iz zad
  //ID TX i RX poruka
  canTx.can_id = 0x100;
  canRx.can_id = 0x200;

  //duljina data u bajtovima
  canTx.can_dlc = 6;
  canRx.can_dlc = 3;

  timerReceiver = 0;
  timerTransmitter = 0;

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);


}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  Serial.println(pot1);
  Serial.println(pot2);


  //frekvencija RX je 10Hz
  if(millis() - timerReceiver > 100){

    timerReceiver = millis();

    if (mcp2515.readMessage(&canRx) == MCP2515::ERROR_OK) {
        //poruka primljen
        bmsState = canRx.data[0];
        //powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
    }
    else{
      //poruka nije primljena
    }
  }

  //frekvencija TX je 20Hz
  if(millis() - timerTransmitter > 50){

    timerTransmitter = millis();

    canTx.data[0] = vcuState;


    //encryptBytes(plausibleRequest, canTx.data[1], canTx.data[2]);
    canTx.data[3] = mapMode;
    //encryptBytes(powerRequest, canTx.data[4], canTx.data[5]);
    auto err = mcp2515.sendMessage(&canTx);

    if(err == MCP2515::ERROR_OK){
      //poruka poslana
    }
    else{
      //poruka nije poslana
    }
  }
  delay(500);

}











