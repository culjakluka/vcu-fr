#include "node.h"

void VcuManageState(NodeState vcu, uint16_t &powerReq, uint16_t plausibleReq){

  if((vcu == ERROR) || (vcu == IDLE)){
    powerReq = 0;
  }

  else{
    //pass to plausibility checker
    //if plausible pass plausible req to mapped req
    //use powerLimiter on mapped req to get final req
  }

}


void VcuChangeState(NodeState bms, bool plausible, NodeState &vcu){

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

}


bool VcuBtnDebounce(int btn){

  static bool btnState = HIGH;
  static bool lastState = HIGH;
  static int debounceTimer = 0;

  bool currState = digitalRead(btn);

  if(currState != lastState){
    debounceTimer = millis();
  }

  //pretpostavljan da je sve ispod 20ms samo sum, a ne zapravo stiskanje botuna
  if((millis() - debounceTimer) > 20){

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





