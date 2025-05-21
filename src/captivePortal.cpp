#include "captivePortal.h"
#include "captiveWebPage.h"

ESPCaptivePortal::ESPCaptivePortal(DeviceState& devState) : server(80) {
    // Constructor implementation (if needed)
}

void ESPCaptivePortal::beginServer() {
    DEBUG_PRINTLN("Starting the captive portal. You can configure ESP32 values using the portal");
    server.begin();
}

void ESPCaptivePortal::endPortal() {
    DEBUG_PRINTLN("Ending the captive portal");
    deviceState.store();
    server.reset();
}

void ESPCaptivePortal::servePortal(bool isAPActive) {
    AsyncElegantOTA.begin(&server); // Start ElegantOTA
    server.on("/cred", HTTP_GET, [this](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("ssid") && request->hasParam("pass")) {
            if (request->hasParam("apn")) {
                PSTATE.apn = request->getParam("apn")->value();
                DEBUG_PRINTF("apn stored %s\t\n", PSTATE.apn.c_str());
            }

            PSTATE.apSSID = request->getParam("ssid")->value();
            DEBUG_PRINTF("ssid stored %s\t\n", PSTATE.apSSID.c_str());
            PSTATE.apPass = request->getParam("pass")->value();
            DEBUG_PRINTF("Pass Stored %s\t\n", PSTATE.apPass.c_str());

            if (request->hasParam("apn")) {
                snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s,\"apn\":%s}", 
                         (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), (PSTATE.apn).c_str());
                request->send(200, "application/json", credResponsePayload);
            } else {
                snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s}", 
                         (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str());
                request->send(200, "application/json", credResponsePayload);
            }
        } else {
            request->send_P(200, "text/html", HTTP_FORM_WIFISET);
        }
    });


    server.onNotFound(_handleNotFound);
    yield();
}

void ESPCaptivePortal::_handleNotFound(AsyncWebServerRequest *request) {
    String message = "File Not Found\n\n";
    request->send(404, "text/plain", message);
}
