#include "node.h"

void pedalValue(int pot1, int pot2, uint16_t plausibleRequest, bool isPlausible){

  float sensorValueDifference = ((float)abs(pot1 - pot2) / 1024) * 100.0;
  //plausibleRequest = ((uint16_t)(senosrValueDifference * 10));
  plausibleRequest = (uint16_t)(((float)(pot1 + pot2)) / 2048.0 * 1000.0);

  static int sensorTimer = 0;

  if(sensorValueDifference > 10){
    isPlausible = false;
    plausibleRequest = 0;
  }

  if(millis() - sensorTimer > 500){
    sensorTimer= millis();
    Serial.print("Plausibility difference: ");
    Serial.print(sensorValueDifference);
    Serial.print("%\t");
    Serial.print("Plausible request: ");
    Serial.println(plausibleRequest);
  }
}


bool checkPlausible(int pot1, int pot2){

  float difference = ((float)abs(pot1 - pot2) / 1024) * 100.0;

  if(difference > 10){
    return false;
  }

  return true;
}


uint16_t getPlausibleReq(int pot1, int pot2){

  if(checkPlausible(pot1, pot2) == true){
    return (uint16_t)(((float)(pot1 + pot2)) * 1000 / 2048);
  }
  return 0;
}



