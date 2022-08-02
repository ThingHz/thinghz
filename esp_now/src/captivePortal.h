#ifndef CAPTIVE_H_
#define CAPTIVE_H_

#include <pgmspace.h>
#include "ESPAsyncWebServer.h"
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);


class ESPCaptivePortal
{
  public:
    // Warning:: requies device state to  be global and longer living
    // than this
    ESPCaptivePortal() {
    }

    /**
       @brief:
       begins the Async WebServer
    */
    void beginServer() {
      AsyncElegantOTA.begin(&server);    // Start ElegantOTA
      DEBUG_PRINTLN("Starting the captive portal. You can configure ESp32 values using portal");
      server.begin();
    }

    /**
       @brief:
       Kills the Async WebServer
    */
    void endPortal() {
      DEBUG_PRINTLN("Ending the captive portal");
      server.reset();
    }

    /**
       @brief:
       Helper funtion for unexpected error
       @param:
       AsyncWebServerRequest
    */
    static void _handleNotFound(AsyncWebServerRequest *request)
    {
      String message = "File Not Found\n\n";
      request->send(404, "text/plain", message);
    }

};


#endif
