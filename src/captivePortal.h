#ifndef ESP_CAPTIVE_PORTAL_H_
#define ESP_CAPTIVE_PORTAL_H_


#include "deviceState.h"
#include "utils.h"
#include "hardwareDefs.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


#define RESPONSE_LENGTH 200

class ESPCaptivePortal {
public:
    ESPCaptivePortal(DeviceState& devState);
    void beginServer();
    void endPortal();
    void servePortal(bool isAPActive);

private:
    static void _handleNotFound(AsyncWebServerRequest *request);
    AsyncWebServer server;
    char credResponsePayload[RESPONSE_LENGTH];
};

#endif // ESP_CAPTIVE_PORTAL_H_
