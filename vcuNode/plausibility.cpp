#include"node.h"

void pedalValue(int pot1, int pot2, int plausibleRequest, bool isPlausible){

  float senosrValueDifference = ((float)abs(pot1 - pot2) / 1024) * 100.0;
  plausibleRequest = ((int)(senosrValueDifference * 100));

  static int sensorTimer = 0;

  if(senosrValueDifference > 10){
    isPlausible = false;
    plausibleRequest = 0;
  }

  if(millis() - sensorTimer > 500){
    sensorTimer= millis();
    Serial.print("Plausible request: ");
    Serial.print(senosrValueDifference);
    Serial.println("%");
  }
}