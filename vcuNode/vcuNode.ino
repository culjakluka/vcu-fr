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
#define ENABLE_BMS_TIMEOUT_FAILSAFE 0
static const unsigned long BMS_TIMEOUT_MS = 300; // > 10Hz period (100ms)
unsigned long lastBmsRxMs = 0;


void setup() {
  Serial.begin(9600);
  SPI.begin();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);

  //ovo je mod za testiranje, inace je setNormalMode() u komunikaciji
  mcp2515.setNormalMode();

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
}

void loop() {

  pot1 = analogRead(A0);
  pot2 = analogRead(A1);

  // FEATURE 1: CAN_RX (10 Hz): citaj BMS poruku 0x100 
  if (millis() - timerReceiver > 100) {
    timerReceiver = millis();

    if (mcp2515.readMessage(&canRx) == MCP2515::ERROR_OK) {
      // filtriraj: prihvati samo CAN_RX (0x100)
      if ((canRx.can_id & 0x7FF) == CAN_RX) {
        bmsState = (NodeState)canRx.data[0];
        powerLimit = decodeBytes(canRx.data[1], canRx.data[2]);
        lastBmsRxMs = millis(); // <-- biljezi kada je zadnji put dosao svjezi limit
      }
    }
  }

#if ENABLE_BMS_TIMEOUT_FAILSAFE
  // Ako BMS ne salje svjez limit dovoljno dugo -> fail-safe (0W)
  if (lastBmsRxMs == 0 || (millis() - lastBmsRxMs) > BMS_TIMEOUT_MS) {
    powerLimit = 0;
    bmsState = ERROR; // opcionalno; moze i IDLE
  }
#endif

  // FEATURE 4: Power Limiter 
  // privremeno: mappedRequest iz pot1 dok ostali feature-i nisu gotovi
  uint16_t mappedRequestPct10 = map(pot1, 0, 1023, 0, 1000); // 0..1000 (0.1%)
  uint16_t requestedPowerW = pct10_to_powerW(mappedRequestPct10);

  // clamp na BMS limit (W)
  powerRequest = applyPowerLimit(requestedPowerW, powerLimit);
  analogWrite(ledPowerRequest, powerW_to_pwm(powerRequest));

#if DEBUG_POWER_LIMITER
  // debug ispis (svakih 200 ms)
  static unsigned long tDebug = 0;
  if (millis() - tDebug > 200) {
    tDebug = millis();
    Serial.print("pot1=");
    Serial.print(pot1);
    Serial.print(" mappedPct10=");
    Serial.print(mappedRequestPct10);
    Serial.print(" requestedW=");
    Serial.print(requestedPowerW);
    Serial.print(" limitW=");
    Serial.print(powerLimit);
    Serial.print(" finalW=");
    Serial.println(powerRequest);
  }
#endif

  // TX (20 Hz) 
  if (millis() - timerTransmitter > 50) {
    timerTransmitter = millis();

    canTx.data[0] = (uint8_t)vcuState;
    encryptBytes(plausibleRequest, canTx.data[1], canTx.data[2]);
    canTx.data[3] = (uint8_t)mapMode;
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
  firstbyte = 10011010
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
