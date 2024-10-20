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
#include "esp_wifi.h"

#include <AsyncTCP.h>
#include "C:\Users\micro\OneDrive\Documents\Arduino\libraries\ESPAsyncWebSrv\src\ESPAsyncWebSrv.h"

// My local files
#include "local/Abstract.h"
#include "local/SSIDs.h"

#include "fwupdate.h"
#include "messages.h"
// Display object to share the MD_Parola class 
LedMatrix ledDisplay;

int REBOOT_MINUTES=0;

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
  int REBOOT_MINUTES =  timeinfo.tm_min;
  DEBUGVAL(REBOOT_MINUTES)
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

int connected=1;

// Web server
AsyncWebServer server(80);

// Web server call back
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
// Web home page
void homePage(AsyncWebServerRequest *request) {
    request->send(200, "text/html", String("\
      <!DOCTYPE html>\
      <html>\
      <head><title>")+String(THINGNAME)+String(" ")+String(VERSION)+String("</title></head>\
      <body>\
      <h2>configuration</h2>\
      <form action=\"/config\">\
      <label for=\"fname\">Your network (RSSI):</label><br>\
      <input type=\"text\" id=\"fname\" name=\"fname\"><br>\
      <label for=\"lname\"Network key (password)</label><br>\
      <input type=\"password\" id=\"lname\" name=\"lname\"><br><br>\
      <input type=\"submit\" value=\"Validate\">\
      </form> \
      </body></html>"));
}
// confic page
void configCallPage(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "reboot");
    ledDisplay.displayString(MSG_REBOOT);
    delay(5000);
    for (int i=5; i>=0; i--){
      ledDisplay.alignment(PA_CENTER);
      ledDisplay.displayString(String(i));
      delay(1000);
    }
    delay (1000);
    ESP.restart();
}

int staCount=0;

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  DEBUG("WiFiStationConnected");
  staCount++;
}

void WiFiStationDisConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  DEBUG("WiFiStationConnected");
  staCount++;
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(WiFiStationConnected,    WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(WiFiStationDisConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);


  // Write firmware name & version
  ledDisplay.alignment(PA_LEFT);
  ledDisplay.displayString(THINGNAME);
  delay (1000);
  ledDisplay.displayString(VERSION);
  delay (1000);

 
  // Try connect wifi
  WiFi.mode(WIFI_MODE_APSTA);
  WiFiMulti.addAP(PRIMARY_SSID);
  #ifdef SECONDARY_SSID
    WiFiMulti.addAP(SECONDARY_SSID);
  #endif

  // wait for WiFi connection
  DEBUG("Serach WIFI");
  ledDisplay.alignment(PA_CENTER);
  ledDisplay.displayString(MSG_SEARCH_WIFI);
  DEBUG("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    ledDisplay.displayString(MSG_CONFIG_MODE);
    connected=0;
  }
  if (connected){
    // Display SSID
    ledDisplay.displayString(WiFi.SSID());
    DEBUGVAL(WiFi.SSID());
    delay (1000);
    DEBUGVAL(WiFi.localIP())
    // Dixplay RSSI
    String srssi=String(WiFi.RSSI());
    srssi+="db";
    ledDisplay.displayString(srssi);
    DEBUGVAL(srssi);
    delay (1000);
    
    DEBUG("Connected");

    // Get time from internet
    ledDisplay.displayString(MSG_GET_TIME_FROM_NTP);
    setClock();  

    // Check avalable new FW
    checkFWUpdate(&ledDisplay);
    
    // Now where am I?
    ledDisplay.displayString(MSG_GET_LOCALES);
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
  } // end of job done if connected
  WiFi.softAP(CONFIG_SSID, CONFIG_PWD);
  DEBUG("AP MODE ENABLED");
  server.on("/", HTTP_GET,homePage);
  server.on("/config",HTTP_GET,configCallPage);
  server.onNotFound(notFound);
  server.begin();
  DEBUGVAL(WiFi.localIP())
  }

void displayTime(char* timeBuffer){
  ledDisplay.alignment(PA_CENTER);
  ledDisplay.displayString(timeBuffer); // display time  
}


int countConnected(){
    return (staCount);
}


void loop() {
  if (connected){
    struct tm timeinfo;
    time_t nows = time(nullptr)+timeShift;
    gmtime_r(&nows, &timeinfo);
    DEBUGVAL(asctime(&timeinfo));
    int h = timeinfo.tm_hour;
    int m =  timeinfo.tm_min;
    if ((h==4) && (m==REBOOT_MINUTES)){
      delay (10000);
      ledDisplay.displayString(MSG_REBOOT);
      delay (50000);
      ESP.restart();
    }
    char timeBuffer[6];
    sprintf(timeBuffer,"%d:%02d", h, m);

    int oldConnected=countConnected();
    if (oldConnected>0)
      ledDisplay.displayString(MSG_WARNING_CONNECTIONS);
    else
      displayTime (timeBuffer);

    
    unsigned long old=m;
    // Wait for next minute, check connected device
    while (old==m) {
      // while current min has not changed
      nows = time(nullptr)+timeShift;
      gmtime_r(&nows, &timeinfo);
      m =  timeinfo.tm_min;

      // if connected display warning message
      int clients=countConnected();
      if (clients!=oldConnected){
        DEBUGVAL(clients);
        oldConnected=clients;
        if (clients>0)
          ledDisplay.displayString(MSG_WARNING_CONNECTIONS);
        else
          displayTime (timeBuffer);
        delay (500);
      }
    }
  } else {
    delay(10);
  }
}


  
