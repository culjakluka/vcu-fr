#include "node.h"
#include <math.h>

//Funkcija za inicijalizaciju svih pinova i varijabli koje se koriste u ostalim funkcijama
//ovo je vec napravljeno unutar setup()
// void initMapping(int ledPin, int btnPin){

//   //Spremanje pinova u lokalne varijable        
//   led = ledPin;                                      
//   btn = btnPin;

//   //Postavljanje pinova
//   pinMode(led, OUTPUT);
//   pinMode(btn, INPUT_PULLUP);                        //PULLUP -> HIGH = NIJE PRITISNUTO, LOW = PRITISNUTO             

//   //Default mode
//   currentMode = NORMAL;

//   //Citanje stanja gumba i spremanje u lokalnu varijablu                    
//   buttonState = digitalRead(btn);

//  //Postavljanje LED diode u pocetno stanje
//   updateLED();                                      
// }

//Funkcija mijenja mapu na pritisak gumba i poziva funkciju za updateanje LED diode
void updateMapping(int btn, Mode &currentMode, int led){
  static bool buttonState = HIGH;
  //Citanje stanja gumba nakon cega se usporeduje sa zadnjim citanjem gumba                             
  bool currentState = digitalRead(btn);    

  if((buttonState == HIGH) && (currentState == LOW)){ 
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
    updateLED(currentMode, led);
    
  }
  buttonState = currentState;
}

//Funkcija mijenja stanje LED diode ovisno o trenutnoj mapi
void updateLED(Mode currentMode, int led){
  switch(currentMode){
    case ECO:
      analogWrite(led, 0);
      break;
    case NORMAL:
      analogWrite(led, 100);
      break;
    case SPORT:
      analogWrite(led, 255);
      break;
  }
}


//Funkcija za primjenjivaje mapa
uint16_t applyMapping(uint16_t plausibleRequest, Mode currentMode){
  if(plausibleRequest == 0){
    return 0;
  }

  uint16_t mappedRequest;
  double mapped;
  double norm = plausibleRequest / 1000.0;

  switch(currentMode){
    case ECO:
      mapped = log(norm + 1) / log(2);
      break;
    case NORMAL:
      mapped = norm;
      break;
    case SPORT:
      //mapped = pow(norm, 2);
      mapped = (exp(norm) - 1) / (M_E - 1);
      break;
  }

  mappedRequest = (uint16_t)(mapped * 1000);

  return mappedRequest;
}



