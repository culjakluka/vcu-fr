#include <SPI.h>
#include <mcp2515.h>

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

struct can_frame canMsg;

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
  canMsg.can_id = CAN_RX;

  //duljina data unutar okvira sta se salje, u bajtovima (svako polje je 1 bajt)
  canMsg.can_dlc = 3;

  timerReceiver = 0;
  timerTransmitter = 0;

}

void loop() {

  if(millis() - timerTransmitter >= 100){

    timerTransmitter = millis();

    canMsg.data[0] = IDLE;
    encryptBytes(powerRequest, canMsg.data[1], canMsg.data[2]);
    auto err = mcp2515.sendMessage(&canMsg);

    if(err == MCP2515::ERROR_OK){
      //poruka poslana
    }
    else{
      //poruka nije poslana
    }
  }


  if(millis() - timerReceiver > 100){

    timerReceiver = millis();

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
        //poruka primljena
        bmsState = canMsg.data[0];
        powerLimit = decodeBytes(canMsg.data[1], canMsg.data[2]);
    }
    else{
      //poruka nije primljena, moze se racunat ka ERROR state od BMS-a
      bmsState = ERROR;
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


