#ifndef CAPTIVE_H_
#define CAPTIVE_H_

#include <pgmspace.h>
#include "deviceState.h"
#include "utils.h"
#include "hardwareDefs.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#define  RESPONSE_LENGTH 200
#include <AsyncElegantOTA.h>

AsyncWebServer server(80);


char responsePayload[RESPONSE_LENGTH];

char checkResponsePayload[RESPONSE_LENGTH];

char credResponsePayload[RESPONSE_LENGTH];

char callResponsePayload[RESPONSE_LENGTH];

const char HTTP_FORM_WIFISET[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>ThingHz</title>
	
<style>body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>
<body><center>
	<h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">ThingHz</h1>
	<br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Configure Device Settings</label><br><br>
	<FORM action="/cred" method= "get">
		<P><label style="font-family:Times New Roman">WiFi SSID</label><br><input maxlength="30px" type = "text" name="ssid" id="ssid" placeholder= "SSID" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;>
		<br><label style="font-family:Times New Roman">WiFi Password</label><br><input maxlength="30px" type = "text" name="pass" id="pass" placeholder= "Password" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;><br>
		<br><label style="font-family:Times New Roman">APN</label><br><input maxlength="30px" type = "text" name="apn" id="apn" placeholder= "APN" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box"><br>
    </P>
		<INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
	</FORM>
</center></body></html>

		)rawliteral";


const char HTTP_FORM_SET_CALLIBRATION_FACTOR[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8" />
    <style>body{ background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style>
    </head>
    <title>ThingHz</title><body>
    <div class="container" align="center" ><center><br>
      <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">ThingHz</h1>
      <FORM action="/call" method= "get">
        <h3 style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;padding-bottom: 20px;text-align: center;font-size: 20px">Set Calibration</h3>
        <P><label style="font-family:Times New Roman">Temperature Calibration</label><br><input maxlength="30px" type = "text" name="tempcal" id="tempcal" placeholder= "Temperature calibration" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box">
        <br><label style="font-family:Times New Roman">Humid Calibration</label><br><input maxlength="30px" type = "text" name="humidcal" id="humidcal" placeholder= "Humidity Calibration" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" >
        <br><label style="font-family:Times New Roman">Light Calibration</label><br><input maxlength="30px" type = "text" name="lightcal" id="lightcal" placeholder= "Light calibration" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box"><br>
        </P>
        <INPUT type="submit" > <style>input[type="submit"]{background-color: #3498DB;border: none;color: white;padding:10px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 12px;}</style></div>
      </FORM>
    </div>
    </body>
    </html>
      )rawliteral";

class ESPCaptivePortal
{
  public:
    // Warning:: requies device state to  be global and longer living
    // than this
    ESPCaptivePortal(DeviceState& devState) {
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
      deviceState.store();
      server.reset();
    }

    /**
       @brief:
       Serves the portal
       @param:
       AP active flag
    */
    void servePortal ( bool isAPActive ) {

      server.on("/cred", HTTP_GET, [](AsyncWebServerRequest * request) {
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
            snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s,\"apn\":%s}", (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), (PSTATE.apn).c_str());
            request->send(200, "application/json", credResponsePayload);
          } else {
            snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s}", (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str());
            request->send(200, "application/json", credResponsePayload);
          }
        } else {
          request->send_P(200, "text/html", HTTP_FORM_WIFISET);
        }
      });

      server.on("/call", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0) {
          if (request->hasParam("tempcal")) {
            PSTATE.tempCalibration = (request->getParam("tempcal")->value()).toInt();
            DEBUG_PRINTF("temp calibration stored %d\t\n", PSTATE.tempCalibration);
          }
          if (request->hasParam("humidcal")) {
            PSTATE.humidCalibration = (request->getParam("humidcal")->value()).toInt();
            DEBUG_PRINTF("humid calibration stored %d\t\n", PSTATE.humidCalibration);
          }
          if (request->hasParam("")) {
            PSTATE.lightCalibration = (request->getParam("lightcal")->value()).toInt();
            DEBUG_PRINTF("light calibration stored %d\t\n", PSTATE.lightCalibration);
          }
        
          
          snprintf(callResponsePayload, RESPONSE_LENGTH, "{\"tempCalibration\":%d,\"humidCalibration\":%d,\"lightCalibration\":%d}", PSTATE.tempCalibration,PSTATE.humidCalibration,PSTATE.lightCalibration);
           request->send(200, "application/json", callResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_SET_CALLIBRATION_FACTOR);
        }
      });

      server.onNotFound(_handleNotFound);
      yield();
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
