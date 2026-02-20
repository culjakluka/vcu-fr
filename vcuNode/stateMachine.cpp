#include "node.h"

void VcuManageState(NodeState vcu, uint16_t &powerReq, uint16_t plausibleReq){

  if((vcu == ERROR) || (vcu == IDLE)){
    powerReq = 0;
  }

  else{
    //nekuzin sta znaci da passa na plausibility checker
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

  static int btnState = HIGH;
  static int lastState = HIGH;
  static int debounceTimer = 0;

  int currState = digitalRead(btn);

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

  lastState = btnState;

  return false;
}





