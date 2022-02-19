#ifndef WIFI_OTA_H
#define WIFI_OTA_H
#include "hardwareDefs.h"
#include <SPIFFS.h>
#include <Update.h>
#include "deviceState.h"


const IPAddress apIP(192, 168, 4, 1);
const IPAddress netMsk(255, 255, 255, 0);

/**
   @brief:
   Function to connect to WiFi to wifi if already connected returns true
   if not then check it the station connected to AP or in portal mode and return true
   or else connectes to the WiFiSTA if not connected
   @param:
   SSID     PassKey     maxDelayRetry       AP-SSID
   @return:
   true if connected to STA or AP
   false in case of faliure
*/


bool reconnectWiFi(const String& ssid, const String& pass, int maxDelay) {
  if (RSTATE.isPortalActive) {
    return true;
  }
  bool connectSuccess = true;
  int loopCounter = 0;
  if (WiFi.isConnected()) {
    DEBUG_PRINTLN("already in STA mode and is connected");
    RSTATE.isAPActive = false;
    return true;
  }
  WiFi.mode(WIFI_STA);  // fixem:: why AP STA mandeep had it like this.
  DEBUG_PRINTF("ssid: %s", ssid.c_str());
  DEBUG_PRINTF("pass: %s", pass.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
  DEBUG_PRINTLN("staring wait for connection\n");
  while (WiFi.status() != WL_CONNECTED)
  {
    DEBUG_PRINT(".");
    delay(maxDelay);
    if (loopCounter == 30) {
      DEBUG_PRINTLN("timeout trying to connect to wifi\n");
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

bool APConnection(const String& APssid) {
  WiFi.disconnect();
  if (WiFi.softAPgetStationNum() > 0 && PSTATE.isOtaAvailable == 0) {
    DEBUG_PRINTLN("Station Connected to SoftAP, keeping soft AP alive");
    RSTATE.isAPActive = true;
    return true;
  }
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(APssid.c_str())) {
    return false;
  }
  delay(100);
  if (!WiFi.softAPConfig(apIP, apIP, netMsk)) {
    return false;
  }
  delay(100);
  DEBUG_PRINT("Connected to: ");
  Serial.println(WiFi.softAPIP());
  RSTATE.isAPActive = true;
  return true;
}


/**
   @brief:
   OTA helper funcltion when the ota is initialised gets the file from SPIFF and initiate OTA
   @param:
   OTA filr name
   @return:
   true if successful OTA
   false in case of faliure
*/

bool writeOTA(const String& filename) {

  File file = SPIFFS.open(filename, "r");
  DEBUG_PRINTF("opening file %s:\t\n", filename.c_str());
  if (!file) {
    DEBUG_PRINTLN("Failed to open file for reading");
    return false;
  }
  DEBUG_PRINTLN("Starting update..");
  size_t fileSize = file.size();
  DEBUG_PRINTF("sizeof the file to be updated %d\t\n", fileSize);
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
  DEBUG_PRINTF("successful update with File Name: %s\n", filename.c_str());
  return true;
}

/**
   @brief:
   Helper function to form ota file name
   @param:
   ota file path        OTA File name
   @return:
   returns fully resolved filepath with name "/fileName"
*/

String fullyResolvedFilePath(const String& path, const String& filename)
{
  String retValue = path;
  retValue += filename;
  return retValue;
}

/**
   @brief:
   get the macAddress without colons for device Id
   @return:
   String to be used as a device Id
*/

String macAddrWithoutColons()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[13] = { 0 };
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}


#endif
