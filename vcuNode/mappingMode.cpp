#include "node.h"
#include <math.h>

static Mode currentMode;
static bool buttonState;
static int led, btn;

//Funkcija za inicijalizaciju svih pinova i varijabli koje se koriste u ostalim funkcijama
void initMapping(int ledPin, int btnPin){

  //Spremanje pinova u lokalne varijable        
  led = ledPin;                                      
  btn = btnPin;

  //Postavljanje pinova
  pinMode(led, OUTPUT);
  pinMode(btn, INPUT_PULLUP);                        //PULLUP -> HIGH = NIJE PRITISNUTO, LOW = PRITISNUTO             

  //Default mode
  currentMode = NORMAL;

  //Citanje stanja gumba i spremanje u lokalnu varijablu                    
  buttonState = digitalRead(btn);

 //Postavljanje LED diode u pocetno stanje
  updateLED();                                      
}

//Funkcija mijenja mapu na pritisak gumba i poziva funkciju za updateanje LED diode
void updateMapping(){
  //Citanje stanja gumba nakon cega se usporeduje sa zadnjim citanjem gumba                             
  bool currentState = digitalRead(btn);              
  if(buttonState == HIGH && currentState == LOW){ 
    switch(currentMode){
      case ECO:
        currentMode = NORMAL;
        break;
      case NORMAL:
        currentMode = SPORT;
        break;
      case SPORT:
        currentMode = ECO;
        break;
    }
    updateLED();
  }
  buttonState = currentState;
}

//Funkcija mijenja stanje LED diode ovisno o trenutnoj mapi
void updateLED(){
  switch(currentMode){
    case ECO:
      analogWrite(led, 80);
      break;
    case NORMAL:
      analogWrite(led, 160);
      break;
    case SPORT:
      analogWrite(led, 255);
      break;
  }
}

//Funkcija za primjenjivaje mapa
uint16_t applyMapping(uint16_t plausibleRequest){
  if(plausibleRequest == 0){
    return 0;
  }

  uint16_t mappedRequest;
  double mapped;
  double norm = plausibleRequest / 1000.0;

  switch(currentMode){
    case ECO:
      mapped = log(norm);
      break;
    case NORMAL:
      mapped = norm;
      break;
    case SPORT:
      mapped = pow(norm, 2);
      break;
  }

  mappedRequest = (uint16_t)(mapped * 1000);

  return mappedRequest;
}