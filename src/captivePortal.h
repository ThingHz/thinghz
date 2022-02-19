#ifndef CAPTIVE_H_
#define CAPTIVE_H_

#include <pgmspace.h>
#include "deviceState.h"
#include "utils.h"
#include "hardwareDefs.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#define  RESPONSE_LENGTH 200

AsyncWebServer server(80);


char responsePayload[RESPONSE_LENGTH];

char correcResponsePayload[RESPONSE_LENGTH];

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
		<br><label style="font-family:Times New Roman">Device ID</label><br><input maxlength="30px" type = "text" name="device" id="device" placeholder= "deviceId" style="width: 400px; padding: 5px 10px ; margin: 8px 0; border : 2px solid #3498DB; border-radius: 4px; box-sizing:border-box" required;><br>
    </P>
		<INPUT type="submit"><style>input[type="submit"]{background-color: #3498DB; border: none;color: white;padding:15px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}</style><br><br>
	</FORM>
</center></body></html>

		)rawliteral";

const char HTTP_FORM_SET_RANGE_FACTOR[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8" />
  <style>body{ background-color: #0067B3  ; font-family: Arial, Helvetica, Sans-Serif }</style>
  </head><title>ThingHz</title><body><div class="container" align="center" ><center><br>
    <h1 style="color:#ffffff; font-family:Times New Roman,Times,Serif;padding-top: 10px;padding-bottom: 5px;font-size: 70px;font-style: oblique">ThingHz</h1>
    <FORM action="/alarm" method= "get">
      <h3 style="color:#FFFFFF;font-family:Times New Roman,Times,Serif;padding-bottom: 20px;text-align: center;font-size: 20px">Alarm Condition</h3>
          <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">Acceptable Temperature Range</label><br><br>
          <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">max</label>&nbsp&nbsp&nbsp&nbsp
		  <select name="tMax" id="tMax_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px;border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
            
            <option value="20">20&#8451 </option>
            <option value="19">19&#8451 </option>
            <option value="18">18&#8451 </option>
            <option value="17">17&#8451 </option>
            <option value="16">16&#8451 </option>
            <option value="15">15&#8451 </option>
			      <option value="14">14&#8451 </option>
            <option value="13">13&#8451 </option>
            <option value="12">12&#8451 </option>
            <option value="11">11&#8451 </option>
			      <option value="10">10&#8451 </option>
            <option value="9">9&#8451 </option>
            <option value="8">8&#8451 </option>
            <option value="7">7&#8451 </option>
            <option value="6">6&#8451 </option>
            <option value="5">5&#8451 </option>
            <option value="4">4&#8451 </option>
            <option value="3">3&#8451 </option>
            <option value="2">2&#8451 </option>
            <option value="1">1&#8451 </option>
            <option value="0">0&#8451 </option>
          </select>&nbsp&nbsp&nbsp&nbsp 
		  <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">min</label>&nbsp&nbsp&nbsp&nbsp
		  <select name="tMin" id="tMin_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px;border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
            <option value="15">15&#8451 </option>
			      <option value="14">14&#8451 </option>
            <option value="13">13&#8451 </option>
            <option value="12">12&#8451 </option>
            <option value="11">11&#8451 </option>
			      <option value="10">10&#8451 </option>
            <option value="9">9&#8451 </option>
            <option value="8">8&#8451 </option>
            <option value="7">7&#8451 </option>
            <option value="6">6&#8451 </option>
            <option value="5">5&#8451 </option>
            <option value="4">4&#8451 </option>
            <option value="3">3&#8451 </option>
            <option value="2">2&#8451 </option>
            <option value="1">1&#8451 </option>
            <option value="0">0&#8451 </option>
            <option value="-1">-1&#8451 </option>
            <option value="-2">-2&#8451 </option>
            <option value="-3">-3&#8451 </option>
            <option value="-4">-4&#8451 </option>
            <option value="-5">-5&#8451 </option>
			      <option value="-6">-6&#8451 </option>
            <option value="-7">-7&#8451 </option>
            <option value="-8">-8&#8451 </option>
            <option value="-9">-9&#8451 </option>
            <option value="-10">-10&#8451 </option>
            <option value="-1">-11&#8451 </option>
            <option value="-2">-12&#8451 </option>
            <option value="-3">-13&#8451 </option>
            <option value="-4">-14&#8451 </option>
            <option value="-5">-15&#8451 </option>
			      <option value="-6">-16&#8451 </option>
            <option value="-7">-17&#8451 </option>
            <option value="-18">-18&#8451 </option>
            <option value="-19">-19&#8451 </option>
            <option value="-20">-20&#8451 </option>
          </select><br><br><br>
          <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">Acceptable Humidity Range</label><br><br>
		  <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">max</label>&nbsp&nbsp&nbsp&nbsp
          <select name="hMax" id="hMax_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px; border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
            <option value="20">20% </option>
            <option value="19">19% </option>
            <option value="18">18% </option>
            <option value="17">17% </option>
            <option value="16">16% </option>
            <option value="15">15% </option>
			      <option value="14">14% </option>
            <option value="13">13% </option>
            <option value="12">12% </option>
            <option value="11">11% </option>
			      <option value="10">10% </option>
            <option value="9">9% </option>
            <option value="8">8% </option>
            <option value="7">7% </option>
            <option value="6">6% </option>
            <option value="5">5% </option>
            <option value="4">4% </option>
            <option value="3">3% </option>
            <option value="2">2% </option>
            <option value="1">1% </option>
          </select>&nbsp&nbsp&nbsp&nbsp
		  <label style="color:#FFFFFF;font-family:Courier New;padding-bottom: 10px;text-align: justify;font-size: 18px">min</label>&nbsp&nbsp&nbsp&nbsp
		  <select name="hMin" id="hMin_id" style="border:2px;  padding: 5px 100px; display: inline-block; margin-top:5px; border: 2px solid #3498DB; border-radius: 4px;box-sizing: border-box;">
            <option value="20">20% </option>
            <option value="19">19% </option>
            <option value="18">18% </option>
            <option value="17">17% </option>
            <option value="16">16% </option>
            <option value="15">15% </option>
			      <option value="14">14% </option>
            <option value="13">13% </option>
            <option value="12">12% </option>
            <option value="11">11% </option>
			      <option value="10">10% </option>
            <option value="9">9% </option>
            <option value="8">8% </option>
            <option value="7">7% </option>
            <option value="6">6% </option>
            <option value="5">5% </option>
            <option value="4">4% </option>
            <option value="3">3% </option>
            <option value="2">2% </option>
            <option value="1">1% </option>
          </select><br><br>
		  <INPUT type="submit" > <style>input[type="submit"]{background-color: #3498DB;border: none;color: white;padding:10px 48px;text-align: center;text-decoration: none;display: inline-block;font-size: 12px;}</style></div>
      </div>
      </FORM>  )rawliteral";

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
        if (request->params() > 0 && request->hasParam("ssid") && request->hasParam("pass")) {
          PSTATE.deviceId = request->getParam("device")->value();
          DEBUG_PRINTF("device stored %s\t\n", PSTATE.deviceId.c_str());
          PSTATE.apSSID = request->getParam("ssid")->value();
          DEBUG_PRINTF("ssid stored %s\t\n", PSTATE.apSSID.c_str());
          PSTATE.apPass = request->getParam("pass")->value();
          DEBUG_PRINTF("Pass Stored %s\t\n", PSTATE.apPass.c_str());
          snprintf(credResponsePayload, RESPONSE_LENGTH, "{\"apSSID\":%s,\"apPass\":%s,\"deviceId\":%s}", (PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(),(PSTATE.deviceId).c_str());
          request->send(200, "application/json", credResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_WIFISET);
        }
      });

      server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest * request) {
        if (request->params() > 0 && request->hasParam("tMin") && request->hasParam("tMax") && request->hasParam("hMin") && request->hasParam("hMax")) {
          PSTATE.targetTempMin = (request->getParam("tMin")->value()).toInt();
          DEBUG_PRINTF("targetTempMin %d\t\n", PSTATE.targetTempMin);
          PSTATE.targetTempMax = (request->getParam("tMax")->value()).toInt();
          DEBUG_PRINTF("targetTempMax %d\t\n", PSTATE.targetTempMax);
          PSTATE.targetHumidityMin = (request->getParam("hMin")->value()).toInt();
          DEBUG_PRINTF("targetHumidMin %d\t\n", PSTATE.targetHumidityMin);
          PSTATE.targetHumidityMax = (request->getParam("hMax")->value()).toInt();
          DEBUG_PRINTF("targetHumidMax %d\t\n", PSTATE.targetHumidityMax);
          snprintf(correcResponsePayload, RESPONSE_LENGTH, "{\"targetTempMin\":%d,\"targetTempMax\":%d,\"targetHumidMin\":%d,\"targetHumidMax\":%d}",
                   PSTATE.targetTempMin,
                   PSTATE.targetTempMax,
                   PSTATE.targetHumidityMin,
                   PSTATE.targetHumidityMax);
          request->send(200, "application/json", correcResponsePayload);
        } else {
          request->send_P(200, "text/html", HTTP_FORM_SET_RANGE_FACTOR);
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
