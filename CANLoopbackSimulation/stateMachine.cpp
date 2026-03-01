#include "node.h"

void VcuManageState(NodeState vcu, int pot1, int pot2, uint16_t &plausibleReq, uint16_t &powerReq, Mode mappingMode){

  if((vcu == ERROR) || (vcu == IDLE)){
    powerReq = 0;
    plausibleReq = 0;
  }

  else{
    //pass to plausibility checker
    //if plausible pass plausible req to mapped req
    //use powerLimiter on mapped req to get final req
    plausibleReq = getPlausibleReq(pot1, pot2);
    powerReq = applyPowerLimit(pct10_to_powerW(applyMapping(plausibleReq, mappingMode)));
  }

}


NodeState VcuChangeState(NodeState bms, bool plausible, NodeState vcu){

  if(bms == READY){

    if(vcu == ERROR){
      vcu = IDLE;
    }
    else if(vcu == READY){
      vcu = ERROR;
    }
    else{
      vcu = READY;
    }
  }

  if((bms == ERROR) || (plausible == false)){
    vcu = ERROR;
  }

  return vcu;

}

//triba testirat ovo ...
bool VcuBtnDebounce(int btn){

  static bool btnState = HIGH;
  static bool lastState = HIGH;
  static int debounceTimer = 0;

  bool currState = digitalRead(btn);

  if(currState != lastState){
    debounceTimer = millis();
  }

  //pretpostavljan da je sve ispod 20ms samo sum, a ne zapravo stiskanje botuna
  if((millis() - debounceTimer) > 10){

    if(currState != btnState){

      btnState = currState;

      if(btnState == LOW){
        return true;
      }
    }
  }

  lastState = currState;

  return false;
}





