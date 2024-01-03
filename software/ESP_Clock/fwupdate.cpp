#define TRACES
#include "WebApp.h"
#include "config.h"
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include "fwupdate.h"
#include "LedMatrix.h"


LedMatrix* pdisp=NULL;

void update_started() {
  DEBUG("CALLBACK:  HTTP update process started");
}

void update_finished() {
  DEBUG("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  DEBUG("CALLBACK:  HTTP update process");
  if (pdisp)
    pdisp->alignment(PA_CENTER);
  
  int prog=cur*100/total;
  DEBUGVAL(prog);
  String s=String(prog);  
  s=s+" %";
  if (pdisp)
    pdisp->displayString(s);
}  

void update_error(int err) {
  ERROR(err);
  DEBUGVAL(err);
}

void checkFWUpdate (LedMatrix* d){
  pdisp=d;
  d->displayString("Update");
  Serial.println(UPDATE_FW_URL);
  delay(500);
  WiFiClient client;
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);
  t_httpUpdate_return ret = httpUpdate.update(client, UPDATE_FW_URL);
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        DEBUGVAL(httpUpdate.getLastError());
        DEBUGVAL(httpUpdate.getLastErrorString().c_str());
        ERROR("HTTP_UPDATE_FAILED Error");
        d->alignment(PA_CENTER);
        d->displayString("Update");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        WARNING("HTTP_UPDATE_NO_UPDATES");
        d->alignment(PA_CENTER);
        d->displayString("Nothing");
        break;

      case HTTP_UPDATE_OK:
        DEBUG("HTTP_UPDATE_OK");
        d->alignment(PA_CENTER);
        d->displayString("Done.");
        break;
    }
}
