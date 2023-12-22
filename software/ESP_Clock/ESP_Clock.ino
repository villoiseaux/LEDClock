#define TRACES

#include <WiFi.h>
#include <WiFiMulti.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include "common.h"
#include "WebApp.h"
// My local files
#include "local/Abstract.h"
#include "local/SSIDs.h"


#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // 4 blocks
#define CS_PIN 21

#define THINGNAME "Clock"
#define VERSION "0.0.1"   // Basic without Web Conf & FW Update
#define VERSION "0.0.2"   // FW Update
#define VERSION "0.0.3w"   // WIP Web Config


#define UPDATE_FW_URL "http://iot.pinon-hebert.fr/esp_clock/ESP_Clock.ino-" VERSION "-next.bin"

// create an instance of the MD_Parola class
MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

int timeShift=0; // time shift in seconds

// Time update
void setClock() {
  configTime(0, 0, "pool.ntp.org");

  DEBUG("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    DEBUG("tic");
    yield();
    nowSecs = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  DEBUGVAL(asctime(&timeinfo));
}

// Manage Wifi
WiFiMulti WiFiMulti;



String getAbstractApiInfo(){
  WiFiClientSecure *client = new WiFiClientSecure;
   if(client) {
     client -> setCACert(AbstractApirootCACertificate);
       {
       HTTPClient https;
       DEBUG("[HTTPS] begin");
       if (https.begin(*client, ABSTRACT_URL)) {  // Defined in Abstract.h
         DEBUG("[HTTPS] GET");
         // start connection and send HTTP header
         int httpCode = https.GET();
         // httpCode will be negative on error
         if (httpCode > 0) {
           // HTTP header has been send and Server response header has been handled
           DEBUGVAL(httpCode);
           // file found at server
           if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
             String payload = https.getString();
             return payload;
           }
          } else {
           DEBUG("[HTTPS] GET... failed, error: ");
           DEBUGVAL(https.errorToString(httpCode).c_str());
          }
          https.end();
        } else {
          ERROR("[HTTPS] Unable to connect");
        }
      }
    }
  return ("ERROR");
}

void update_started() {
  DEBUG("CALLBACK:  HTTP update process started");
}

void update_finished() {
  DEBUG("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  DEBUG("CALLBACK:  HTTP update process");
  ledMatrix.setTextAlignment(PA_CENTER);
  int prog=cur*100/total;
  DEBUGVAL(prog);
  String s=String(prog);  
  s=s+" %";
  ledMatrix.print(s);
  
}

void update_error(int err) {
  ERROR(err);
  DEBUGVAL(err);
}

void checkFWUpdate (){
  WiFiClient client;
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);
  DEBUG("UPDATE");
  DEBUGVAL(UPDATE_FW_URL);
  t_httpUpdate_return ret = httpUpdate.update(client, UPDATE_FW_URL);
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        DEBUGVAL(httpUpdate.getLastError());
        DEBUGVAL(httpUpdate.getLastErrorString().c_str());
        ERROR("HTTP_UPDATE_FAILED Error");
        ledMatrix.setTextAlignment(PA_CENTER);
        ledMatrix.print("Update");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        WARNING("HTTP_UPDATE_NO_UPDATES");
        ledMatrix.setTextAlignment(PA_CENTER);
        ledMatrix.print("Nothing");
        break;

      case HTTP_UPDATE_OK:
        DEBUG("HTTP_UPDATE_OK");
        ledMatrix.setTextAlignment(PA_CENTER);
        ledMatrix.print("Done.");
        break;
    }
}

void setup() {
  Serial.begin(115200);

  // Initialize the breakout buildin led
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  ledMatrix.begin();         // initialize the LED Matrix
  ledMatrix.setIntensity(0); // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix.displayClear();  // clear LED matrix display

  
  ledMatrix.setTextAlignment(PA_LEFT);
  ledMatrix.print(THINGNAME);
  delay (3000);
  ledMatrix.print(VERSION);
  delay (3000);

 
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(PRIMARY_SSID);
  #ifdef SECONDARY_SSID
    WiFiMulti.addAP(SECONDARY_SSID);
  #endif

  // wait for WiFi connection
  DEBUG("Serach WIFI");
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("WiFi");
  DEBUG("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    ledMatrix.print("Config");
    DEBUGVAL(WebApp::strStatus(WiFi.status()));
    WebApp config("CONFIG");
    config.initWifiAP();
    config.runApp();
    FATAL("Nothing to do here!");
  }
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(WiFi.SSID());
  delay (1000);

  String srssi=String(WiFi.RSSI());
  srssi+="db";
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(srssi);
  DEBUGVAL(srssi);
  delay (1000);
  
  DEBUG("Connected");
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("Time");
  setClock();  

  DEBUG("Connected");
  ledMatrix.print("upd?");
  checkFWUpdate();
  
  
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("Geoloc");
  String loc=getAbstractApiInfo();
  DEBUGVAL(loc);

  JSONVar myObject = JSON.parse(loc);
  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") {
    ERROR("Parsing input failed!");
    return;
  }

  DEBUGVAL(JSON.typeof(myObject)); // prints: object 
  int hShift;
  if (myObject.hasOwnProperty("timezone")) {
    hShift=(int)myObject["timezone"]["gmt_offset"];
    DEBUGVAL(hShift);
  }
  int sShift=hShift*3600;
  
  String timeZone="TU";
  if (sShift>=0)
    timeZone+="+";
  timeZone+=String(hShift);
  ledMatrix.print(timeZone);
  delay (5000);
  timeShift=sShift;
}

void displayTime(char* timeBuffer){
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(timeBuffer); // display time  
}

void loop() {
  struct tm timeinfo;
  time_t nows = time(nullptr)+timeShift;
  gmtime_r(&nows, &timeinfo);
  DEBUGVAL(asctime(&timeinfo));
  int h = timeinfo.tm_hour;
  int m =  timeinfo.tm_min;
  if ((h==04) && (m==00)){
    delay (10000);
    ledMatrix.print("Reboot");
    delay (50000);
    ESP.restart();
  }
  char timeBuffer[6];
  sprintf(timeBuffer,"%d:%02d", h, m);

  displayTime (timeBuffer);
  unsigned long old=m;
  // Wait for next minute
  while (old==m) {
    nows = time(nullptr)+timeShift;
    gmtime_r(&nows, &timeinfo);
    m =  timeinfo.tm_min;
  }
}


  
