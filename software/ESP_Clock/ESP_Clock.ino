#define TRACES

#include <WiFi.h>
#include <WiFiMulti.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include "config.h"
#include "common.h"
#include "WebApp.h"
#include "ledMatrix.h"

// My local files
#include "local/Abstract.h"
#include "local/SSIDs.h"

#include "fwupdate.h"

// Display object to share the MD_Parola class 
LedMatrix ledDisplay;

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


void setup() {
  Serial.begin(115200);


  // Write firmware name & version
  ledDisplay.alignment(PA_LEFT);
  ledDisplay.displayString(THINGNAME);
  delay (3000);
  ledDisplay.displayString(VERSION);
  delay (3000);

 
  // Try connect wifi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(PRIMARY_SSID);
  #ifdef SECONDARY_SSID
    WiFiMulti.addAP(SECONDARY_SSID);
  #endif

  // wait for WiFi connection
  DEBUG("Serach WIFI");
  ledDisplay.alignment(PA_CENTER);
  ledDisplay.displayString("WiFi");
  DEBUG("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    ledDisplay.displayString("Config");
    DEBUGVAL(WebApp::strStatus(WiFi.status()));
    WebApp config("CONFIG");
    config.initWifiAP();
    config.runApp();
    FATAL("Nothing to do here!");
  }
  // Display SSID
  ledDisplay.displayString(WiFi.SSID());
  DEBUGVAL(WiFi.SSID());
  delay (1000);

  // Dixplay RSSI
  String srssi=String(WiFi.RSSI());
  srssi+="db";
  ledDisplay.displayString(srssi);
  DEBUGVAL(srssi);
  delay (1000);
  
  DEBUG("Connected");

  // Get time from internet
  ledDisplay.displayString("Time");
  setClock();  

  // Check avalable new FW
  checkFWUpdate(&ledDisplay);
  
  // Now where am I?
  ledDisplay.displayString("Geoloc");
  String loc=getAbstractApiInfo();
  DEBUGVAL(loc);

  JSONVar myObject = JSON.parse(loc);
  if (JSON.typeof(myObject) == "undefined") {
    ERROR("Parsing input failed!");
    return;
  }

  DEBUGVAL(JSON.typeof(myObject)); // prints: object 
  int hShift;
  String locale;
  if (myObject.hasOwnProperty("timezone")) {
    hShift=(int)myObject["timezone"]["gmt_offset"];
    locale=JSON.stringify(myObject["timezone"]["name"]);
    locale.replace("\"","");
    locale.replace("Europe/","");
    DEBUGVAL(hShift);
  }
  int sShift=hShift*3600;
  
  String timeZone="TU";
  if (sShift>=0)
    timeZone+="+";
  timeZone+=String(hShift);
  ledDisplay.displayString(locale);
  delay (2500);
  ledDisplay.displayString(timeZone);
  delay (2500);
  timeShift=sShift;
}

void displayTime(char* timeBuffer){
  ledDisplay.alignment(PA_CENTER);
  ledDisplay.displayString(timeBuffer); // display time  
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
    ledDisplay.displayString("Reboot");
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


  
