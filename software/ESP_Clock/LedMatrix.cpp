#define TRACES
#include "WebApp.h"
#include "ledMatrix.h"

LedMatrix::LedMatrix(){
  ledMatrix = new MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
  ledMatrix->begin();         // initialize the LED Matrix
  ledMatrix->setIntensity(0); // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix->displayClear();  // clear LED matrix display
}

void LedMatrix::displayString(String stringToDisplay){
  ledMatrix->print(stringToDisplay);
  DEBUGVAL(stringToDisplay)
}

void LedMatrix::alignment(textPosition_t tp){
  ledMatrix->setTextAlignment(tp);
}