#ifndef CAPTIVE_H_
#define CAPTIVE_H_

#include <pgmspace.h>
#include "deviceState.h"
#include "utils.h"
#include "hardwaredefs.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#define  RESPONSE_LENGTH 200

AsyncWebServer server(80);


char responsePayload[RESPONSE_LENGTH];

char correcResponsePayload[RESPONSE_LENGTH];

char checkResponsePayload[RESPONSE_LENGTH];

char credResponsePayload[RESPONSE_LENGTH];

char tempRequestPayload[RESPONSE_LENGTH];

char callResponsePayload[RESPONSE_LENGTH];

const char HTTP_FORM_WIFISET[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>ThingHz</title>
	
<style>body { background-color: #0067B3 ; font-family: Arial, Helvetica, Sans-Serif; Color: #FFFFFF; }input[type=text], select {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}</style></head>
<body><center>
	<h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">ThingHz</h1>
	<br><label style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;font-size: 24px;padding-top: 5px;padding-bottom: 10px;">Configure Device Settings</label><br><br>
	<FORM action="/cred" method= "get">
		<P><label style="font-family:Times New Roman">WiFi SSID</label><br><input maxlength="30px" type = "text" name="ssid" id="ssid" placeholder= "SSID" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;>
		<br><label style="font-family:Times New Roman">WiFi Password</label><br><input maxlength="30px" type = "text" name="pass" id="pass" placeholder= "Password" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;><br>
		<br><label style="font-family:Times New Roman">Device ID</label><br><input maxlength="30px" type = "text" name="device" id="device" placeholder= "deviceId" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box"><br>
    <input type="radio" name="configure" value="wifi" id="radio_wifi" style="color:blue;" > Use as a Wifi device &nbsp&nbsp
    <input type="radio" name="configure" value="ble" id="radio_ble" style="color:blue;">  Use as a BLE Beacon<br>
    </P>
		<INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
	</FORM>
</center></body></html>

		)rawliteral";


const char HTTP_FORM_SET_CALLIBRATION_FACTOR[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8" />
    <style>body{ background-color: #0067B3  ; font-family: Arial, Helvetica, Sans-Serif }</style>
    </head><title>ThingHz</title><body><div class="container" align="center" ><center><br>
      <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">ThingHz</h1>
      <FORM action="/call" method= "get">
        <h3 style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;padding-bottom: 20px;text-align: center;font-size: 20px">Set Callibration</h3>
            <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">Temperature Callibration</label>&nbsp&nbsp&nbsp&nbsp
            <select name="tCall" id="tCall_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px;border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
              <option value="-10">-10&#8451 </option>
              <option value="-9">-9&#8451 </option>
              <option value="-8">-8&#8451 </option>
              <option value="-7">-7&#8451 </option>
              <option value="-6">-6&#8451 </option>
              <option value="-5">-5&#8451 </option>
              <option value="-4">-4&#8451 </option>
              <option value="-3">-3&#8451 </option>
              <option value="-2">-2&#8451 </option>
              <option value="-1">-1&#8451 </option>
              <option value="0">0&#8451 </option>
              <option value="1">1&#8451 </option>
              <option value="2">2&#8451 </option>
              <option value="3">3&#8451 </option>
              <option value="4">4&#8451 </option>
              <option value="5">5&#8451 </option>
              <option value="6">6&#8451 </option>
              <option value="7">7&#8451 </option>
              <option value="8">8&#8451 </option>
              <option value="9">9&#8451 </option>
              <option value="10">10&#8451 </option>
            </select><br><br>
            <INPUT type="submit" > <style>input[type="submit"]{background-color: #3498DB;border: none;color: white;padding:10px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 12px;}</style></div>
        </div>
        </FORM>  )rawliteral";

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
       Serves the portal
       @param:
       AP active flag
    */
    void servePortal ( bool isAPActive ) {

      server.on("/cred", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("ssid") && request->hasParam("pass") && request->hasParam("configure")) {
          if (request->hasParam("device")) {
            PSTATE.deviceId = request->getParam("device")->value();
            DEBUG_PRINTF("device stored %s\t\n", PSTATE.deviceId.c_str());
          }
        
          PSTATE.apSSID = request->getParam("ssid")->value();
          DEBUG_PRINTF("ssid stored %s\t\n", PSTATE.apSSID.c_str());
          PSTATE.apPass = request->getParam("pass")->value();
          DEBUG_PRINTF("Pass Stored %s\t\n", PSTATE.apPass.c_str());
          String bleSelected = request->getParam("configure")->value();
          DEBUG_PRINTF("ble stored %s\t\n", bleSelected.c_str());
          if(bleSelected == "ble"){
              PSTATE.isBLE = 1;
          }else{
              PSTATE.isBLE = 0;
          }
          
          if (request->hasParam("device")) {
            snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s,\"deviceId\":%s,\"isBLE\":%d}", (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), (PSTATE.deviceId).c_str(), PSTATE.isBLE);
            request->send(200, "application/json", credResponsePayload);
          } else {
            snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s,\"isBLE\":%d}", (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), PSTATE.isBLE);
            request->send(200, "application/json", credResponsePayload);
          }
        } else {
          request->send_P(200, "text/html", HTTP_FORM_WIFISET);
        }
      });


      server.on("/call", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("tCall")) {
          PSTATE.tempCalibration = (request->getParam("tCall")->value()).toInt();
          DEBUG_PRINTF("tempCalibration %d\t\n", PSTATE.tempCalibration);
          snprintf(callResponsePayload, RESPONSE_LENGTH, "{\"tempCalibration\":%d}", PSTATE.tempCalibration);
          request->send(200, "application/json", callResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_SET_CALLIBRATION_FACTOR);
        }
      });

      server.on("/check", HTTP_GET, [](AsyncWebServerRequest * request) {
        String isSuccess = "true";
        snprintf(checkResponsePayload, RESPONSE_LENGTH, "{\"Success\":\"%s\",\"DeviceId\":\"%s\"}",
                 isSuccess.c_str(),(PSTATE.deviceId).c_str());
        request->send(200, "application/json", checkResponsePayload);
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
