#ifndef _CLOCK_LEDMATRIX
  #define _CLOCK_LEDMATRIX
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // 4 blocks
#define CS_PIN 21

class LedMatrix{
  MD_Parola *ledMatrix;
  public:
  LedMatrix(); 
  void alignment(textPosition_t tp);
  void displayString(String stringToDisplay);
};

#endif