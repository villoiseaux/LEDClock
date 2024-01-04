#ifndef WEBAPP_H
  #define WEBAPP_H
  #define ASSERT(condition)  {Serial.printf("##[%09d] 0-ASSERT WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.print(String(" condition:'")+#condition+String("' ")); if (condition) {Serial.println("[PASS]");} else {Serial.println("[FAIL]\n################################################# HALT #################################################"); esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR); esp_deep_sleep_start();}}
  
  #ifdef TRACES
    #define FATAL(message)     {Serial.printf("##[%09d] 0-FATAL ERROR WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.println(message); delay(5000);  ESP.restart();}
    #define ERROR(message)     {Serial.printf("##[%09d] 1-ERROR       WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.println(message);}
    #define WARNING(message)   {Serial.printf("##[%09d] 3-WARNING     WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.println(message);}
    #define DEBUG(message)     {Serial.printf("##[%09d] 7-DEBUG TRACE WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.println(message);}
    #define DEBUGVAL(value)    {Serial.printf("##[%09d] 8-LOG VALUE   WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print(String("} ")+#value+String("=")); Serial.println(value);}
    #define DEBUGHEXVAL(value) {Serial.printf("##[%09d] 8-HEX VALUE   WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print(String("} ")+#value+String("=")); Serial.println(value,HEX);}
  #else
    #define FATAL(message)     {Serial.printf("##[%09d] 0-FATAL ERROR WebApp::",millis()); Serial.print(__func__); Serial.print("() {"); Serial.print(__LINE__); Serial.print("} "); Serial.println(message);  delay(5000); ESP.restart();;}
    #define ERROR(message) {}
    #define WARNING(message) {}
    #define DEBUG(message) {}
    #define DEBUGVAL(value) {}
    #define DEBUGHEXVAL(value) {} 
  #endif  
#include <Arduino.h>
#include <WiFi.h>
#endif
