#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define TRACES
#include "WebApp.h"


AsyncWebServer appServer(80);

WebApp::WebApp(String appName, String appversion){
  _appName=appName;
  DEBUG("CREATE WEB APP")
  DEBUGVAL(appName);
}

void WebApp::runApp(){
  appServer.begin();
  DEBUG ("WEB SERVER STARTED");
  DEBUG("[------------------ WIP -------------------]");
  for (;;){
    delay (10000);
    DEBUG("[ Infinite loop ]");
  }
}

WebApp::~WebApp(){}

int WebApp::initWifiAP(){
  DEBUG("Create Wifi AP");
  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP("LED_CLOCK");
  if(result == true){
    DEBUG("AP Ready");
    DEBUGVAL(WiFi.softAPIP());
    return (false);
  } else {
    FATAL("AP Failed!");
  }      
}

String WebApp::strStatus(int st){
      String retStr;
      switch (st){
        case WL_CONNECTED:
          retStr="WIFI STATUS: WL_CONNECTED";
          break;
        case WL_NO_SHIELD:
          retStr="WIFI STATUS: WL_NO_SHIELD";
          break;
        case WL_IDLE_STATUS:
          retStr="WIFI STATUS: WL_IDLE_STATUS";
          break;
        case WL_CONNECT_FAILED:
          retStr="WIFI STATUS: WL_CONNECT_FAILED";
          break;
        case WL_NO_SSID_AVAIL:
          retStr="WIFI STATUS: WL_NO_SSID_AVAIL";
          break;
        case WL_SCAN_COMPLETED:
          retStr="WIFI STATUS: WL_SCAN_COMPLETED";
          break;
        case WL_CONNECTION_LOST:
          retStr="WIFI STATUS: WL_CONNECTION_LOST";
          break;
        case WL_DISCONNECTED:
          retStr="WIFI STATUS: WL_DISCONNECTED";
          break;
        default:
          retStr="UNKNOWN WIFI STATUS";
      }
      return (retStr);
    }
