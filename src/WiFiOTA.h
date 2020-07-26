#ifndef WIFI_OTA_H
#define WIFI_OTA_H
#include "hardwaredefs.h"
#include <SPIFFS.h>
#include <Update.h>
#include "devicestate.h"

/**
 * @brief:
 * Function to connect to WiFi to wifi if already connected returns true
 * if not then check it the station connected to AP or in portal mode and return true
 * or else connectes to the WiFiSTA if not connected
 * @param:
 * SSID     PassKey     maxDelayRetry       AP-SSID
 * @return:
 * true if connected to STA or AP
 * false in case of faliure   
*/


bool reconnectWiFi(const String& ssid, const String& pass, int maxDelay, const String& APssid) {
  bool connectSuccess = true;
  int loopCounter = 0;
  if (WiFi.softAPgetStationNum() > 0 && PSTATE.isOtaAvailable == 0) {
    DEBUG_PRINTLN("Station Connected to SoftAP, keeping soft AP alive");
    RSTATE.isAPActive = true;
    return true;
  }
  if (WiFi.isConnected()) {
    DEBUG_PRINTLN("already in STA mode and is connected");
    RSTATE.isAPActive = false;
    return true;
  }
  
  delay(500);
  WiFi.mode(WIFI_STA);  // fixem:: why AP STA mandeep had it like this.
  DEBUG_PRINTF("ssid: %s", ssid.c_str());
  DEBUG_PRINTF("pass: %s", pass.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
  DEBUG_PRINTLN("staring wait for connection\n");
  delay(500);
  while (WiFi.status() != WL_CONNECTED)
  {
    DEBUG_PRINT(".");
    delay(maxDelay);
    if (loopCounter == 30) {
      DEBUG_PRINTLN("timeout trying to connect to wifi\n");
      RSTATE.isPayloadPostTimeout = false;
      connectSuccess = false;
      break;
    }
    loopCounter++;
  }
  connectSuccess = WiFi.isConnected();
  if (connectSuccess) {
    DEBUG_PRINTLN("connected to:  ");
    DEBUG_PRINTLN(WiFi.localIP());
    RSTATE.isAPActive = false;
  }
  return connectSuccess;
}

/**
 * @brief:
 * OTA helper funcltion when the ota is initialised gets the file from SPIFF and initiate OTA
 * @param:
 * OTA filr name
 * @return:
 * true if successful OTA
 * false in case of faliure   
*/

bool writeOTA(const String& filename) {
    
    File file = SPIFFS.open(filename, "r");
    DEBUG_PRINTF("opening file %s:\t\n",filename);
    if (!file) {
      DEBUG_PRINTLN("Failed to open file for reading");
      return false;
    }
    DEBUG_PRINTLN("Starting update..");
    size_t fileSize = file.size();
    DEBUG_PRINTF("sizeof the file to be updated %d\t\n",fileSize);
    if (!Update.begin(fileSize)) {
      Update.printError(Serial);
      DEBUG_PRINTLN("Cannot do the update");
      file.close();
      return false;
    }
    size_t fileSizeWritten = Update.writeStream(file);
    if (fileSizeWritten != fileSize) {
       DEBUG_PRINTLN("Error While Writing %d" + String(Update.getError()));
       DEBUG_PRINTF("File Size not correct %d\n", fileSizeWritten);
       return false;
    }
    if (Update.end(true)) {
      DEBUG_PRINTLN("Update Successful");
    } else {
      DEBUG_PRINTLN("Error Occurred: " + String(Update.getError()));
      file.close();
      return false;
    }
    file.close();
    DEBUG_PRINTF("Successful Update with filename : %s\n", filename.c_str());
    return true;
  }

/**
 * @brief:
 * Helper function to form ota file name
 * @param:
 * ota file path        OTA File name
 * @return:
 * returns fully resolved filepath with name "/fileName"   
*/

String fullyResolvedFilePath(const String& path, const String& filename)
{
  String retValue = path;
  retValue += filename;
  return retValue;
}

#endif