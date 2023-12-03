#include <WiFi.h>
#include <WiFiMulti.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include "CA-certificat.h"
// My local files
#include "local/Abstract.h"
#include "local/SSIDs.h"


#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // 4 blocks
#define CS_PIN 21

#define THINGNAME "Th CLK"
#define VERSION "0.0.1"   // Basic without Web Conf & FW Update
#define VERSION "0.0.2"   // WIP FW Update


#define UPDATE_FW_URL "http://iot.pinon-hebert.fr/esp_clock/ESP_Clock.ino-" VERSION "-next.bin"

// create an instance of the MD_Parola class
MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

int timeShift=0; // time shift in seconds

// Time update
void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.println("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));
}

// Manage Wifi
WiFiMulti WiFiMulti;



String getAbstractApiInfo(){
  WiFiClientSecure *client = new WiFiClientSecure;
   if(client) {
     client -> setCACert(AbstractApirootCACertificate);
       {
       HTTPClient https;
       Serial.print("[HTTPS] begin...\n");
       if (https.begin(*client, ABSTRACT_URL)) {  // Defined in Abstract.h
         Serial.print("[HTTPS] GET...\n");
         // start connection and send HTTP header
         int httpCode = https.GET();
         // httpCode will be negative on error
         if (httpCode > 0) {
           // HTTP header has been send and Server response header has been handled
           Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
           // file found at server
           if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
             String payload = https.getString();
             return payload;
           }
          } else {
           Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }
          https.end();
        } else {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
    }
  return ("ERROR");
}

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
  ledMatrix.setTextAlignment(PA_CENTER);
  int prog=cur*100/total;
  String s=String(prog);  
  s=s+" %";
  ledMatrix.print(s);
  
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void checkFWUpdate (){
  WiFiClient client;
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);
  Serial.println("UPDATE" UPDATE_FW_URL);
  t_httpUpdate_return ret = httpUpdate.update(client, UPDATE_FW_URL);
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        ledMatrix.setTextAlignment(PA_CENTER);
        ledMatrix.print("Update");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        ledMatrix.setTextAlignment(PA_CENTER);
        ledMatrix.print("Nothing");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
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
  Serial.println("Serach WIFI");
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("WiFi");
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    delay (100);
  }
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(WiFi.SSID());
  delay (1000);
  
  Serial.println(" connected");
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("Time");
  setClock();  

  Serial.println(" connected");
  ledMatrix.print("upd?");
  checkFWUpdate();
  delay (1000);
  
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print("Geoloc");
  String loc=getAbstractApiInfo();
  Serial.println(loc);

  JSONVar myObject = JSON.parse(loc);
  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object 
  int hShift;
  if (myObject.hasOwnProperty("timezone")) {
    hShift=(int)myObject["timezone"]["gmt_offset"];
    Serial.println(hShift);
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

void loop() {
  struct tm timeinfo;
  time_t nows = time(nullptr)+timeShift;
  gmtime_r(&nows, &timeinfo);
  Serial.print(asctime(&timeinfo));
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
  
  ledMatrix.setTextAlignment(PA_CENTER);
  ledMatrix.print(timeBuffer); // display time
  unsigned long old=m;
  // Wait for next minute
  while (old==m) {
    nows = time(nullptr)+timeShift;
    gmtime_r(&nows, &timeinfo);
    m =  timeinfo.tm_min;
  }
}


  
