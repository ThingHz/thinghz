#ifndef CLOUDTALK_H
#define CLOUDTALK_H

#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <SPIFFS.h>
#include "wifiOTA.h"
#include "deviceState.h"
#include "hardwaredefs.h"
#include "utils.h"
#include "SensorPayload.h"
#define JSON_MSG_MAX_LEN          512

DynamicJsonDocument jsonDocument(1024);


//const char urlmessageSend[] = "https://ir989t4sy0.execute-api.us-east-1.amazonaws.com/prod/data";
const char urlmessageSend[] = "https://api.thinghz.com/v1/data";
const char urlOtaSend[]     = "http://3.19.52.97:9955/api/data/download-file?filename=%s"; //URL for ota file download

// payload structure for OTA result update to cloud
const char COOLNEXT_OTA_BODY[] = "{\"deviceId\":\"%s\",\"firmwareVersion\":\"%s\"}";
char otaPayload[JSON_MSG_MAX_LEN];

/**
   @brief: FWInfo class for storing FW related information
*/

class FWInfo {
  public:
    uint8_t fwVersion = 0;
    uint32_t fileSize = 0;
};

class CloudTalk {
  public:
    /**
       @brief: Create http link and send the device payload to cloud
       @return: true when everything works right
    */

    bool sendPayload()
    {
      String retJson;
      int httpCode;
      HTTPClient http;
      http.begin(urlmessageSend);//zx     //test
      http.addHeader("Content-Type" , "application/json");  //Specify content-type header
      String messagePayload = createPayload(DEVICE_SENSOR_TYPE);
      DEBUG_PRINTLN(messagePayload);
      httpCode = http.POST(messagePayload);     //Send the request
      yield();
      if (httpCode == HTTP_CODE_OK && httpCode > 0) {
        retJson = http.getString();   //Get the response payload
        DEBUG_PRINTLN(retJson);
      } else {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        return false;
      }
      http.end();

      FWInfo fwInfo = extractFWVersion(retJson);

      if (fwInfo.fwVersion != PSTATE.newfWVersion) {
        DEBUG_PRINTF("current paylod processed, old fw version : %u, new fw version: %u\n", PSTATE.newfWVersion, fwInfo.fwVersion );

        String remoteFwFilename = prepareRemoteFWFileName(ThingHz_Standalone,
                                  HW_REV,
                                  fwInfo.fwVersion);

        size_t filesize = fetchOTAFile(remoteFwFilename, fwInfo.fileSize);


        if (filesize) {
          DEBUG_PRINTLN("Processing firmware upgrade for self");
          PSTATE.newfWVersion = fwInfo.fwVersion;
          PSTATE.isOtaAvailable = 1;
          deviceState.store();
        }
      }
      return true;
    }

 /**
       @brief: Create message payload
       @param: Sensor profile of sesnor type
       @return: message payload array
    */

    String createPayload(uint8_t sProfile) {
      char messageCreatePayload[JSON_MSG_MAX_LEN];
      switch (sProfile) {
        case SensorProfile::SensorNone :
          DEBUG_PRINTLN("NO Sensor Found");
          break;
        case SensorProfile::SensorTemp :
          PAYLOAD_T.temp = RSTATE.temperature;
          DEBUG_PRINTLN("Creating payload for Temp Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_T.temp,
                   PAYLOAD_T.sensorProfile,
                   RSTATE.batteryPercentage
                  );
          DEBUG_PRINTLN(messageCreatePayload);
          break;
        case SensorProfile::SensorTH :
          PAYLOAD_TH.temp = RSTATE.temperature;
          PAYLOAD_TH.humidity = RSTATE.humidity;
          DEBUG_PRINTLN("Creating payload for Temp Humid Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"humid\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_TH.temp,
                   PAYLOAD_TH.humidity,
                   PAYLOAD_TH.sensorProfile,
                   RSTATE.batteryPercentage
                  );
          break;
        case SensorProfile::SensorTHM :
          DEBUG_PRINTLN("Creating payload for Temp Humid Moist Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"moisture\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_THM.temp,
                   PAYLOAD_THM.humidity,
                   PAYLOAD_THM.moisture,
                   RSTATE.batteryPercentage,
                   PAYLOAD_TH.sensorProfile);

          break;
        case SensorProfile::SensorGas :
          DEBUG_PRINTLN("Creating payload for Gas Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"gas\":\"%u\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                    (PSTATE.deviceId).c_str(),
                   PAYLOAD_GAS.gas,
                   RSTATE.batteryPercentage,
                   PAYLOAD_GAS.sensorProfile);
          break;
        case SensorProfile::SensorGyroAccel :
          DEBUG_PRINTLN("Creating payload for Temp Humid Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"deviceId\":\"%s\",\"gyro\":\"%d\",\"accel\":\"%d\",\"batteryPercentage\":\"%d\",\"sensorProfile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_GA.gyro,
                   PAYLOAD_GA.accel,
                   RSTATE.batteryPercentage,
                   PAYLOAD_TH.sensorProfile);
          break;
        case SensorProfile::SensorTHC:
           DEBUG_PRINTLN("Creating payload for Temp Humid Cap Sensor");
           PAYLOAD_THC.temp = RSTATE.temperature;
           PAYLOAD_THC.humidity = RSTATE.humidity;
           PAYLOAD_THC.capcitance = RSTATE.capacitance;
           snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"cap\":\"%.4f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_THC.temp,
                   PAYLOAD_THC.humidity,
                   PAYLOAD_THC.capcitance,
                   RSTATE.batteryPercentage,
                   PAYLOAD_THC.sensorProfile);
          break;
        default:
          DEBUG_PRINTLN("Not a valid Sensor");
          break;
      }
      return String(messageCreatePayload);
    }

    /**
       @brief: update the ota version during successful update
       @return: true if everything happened as expected
    */

    bool updateOTAversion() {
      //strncpy(devState.rtcState.newfWVersion, fwValue.c_str(), sizeof(devState.rtcState.newfWVersion));
      String fWversion = String(PSTATE.newfWVersion);
      //DEBUG_PRINTLN("New fw Version %s",fWversion);
      //const String& macStr = convertToStringWithoutColons(telemeteryElement->_mac);
      snprintf(otaPayload, JSON_MSG_MAX_LEN, COOLNEXT_OTA_BODY, (PSTATE.deviceId).c_str(), fWversion.c_str());
      HTTPClient http;
      http.begin(urlOtaSend); //SenseNext production
      http.addHeader("Content-Type" , "application/json");  //Specify content-type header
      int otaCode = http.PUT(otaPayload);
      if (otaCode <= 0 ) {
        return false;
      }
      String otaLoad = http.getString();
      DEBUG_PRINTF("ota payload for updation\t%s\n", otaLoad.c_str());
      DEBUG_PRINTLN("version updated");
      PSTATE.isOtaAvailable = 0;
      http.end();
      return true;
    }

    /**
       @brief: Extract the fw version for the response payload
       @return: true if everything happened as expected
    */

    FWInfo extractFWVersion(String load) const
    {
      FWInfo fwInfo;

      if (load == NULL) {
        return fwInfo;
      }


      DeserializationError error = deserializeJson(jsonDocument, load);

      if (error) {
        DEBUG_PRINTLN("failed to deserialize");
        return fwInfo;
      }

      // TODO:: need to think better about this, if false what do we do
      String succ;
      if (!jsonDocument.containsKey("data")) {
        DEBUG_PRINTLN("debug doesn't contain data key, not processing");
        return fwInfo;
      }

      JsonArray requests = jsonDocument["data"];
      if (requests.size() == 0) {
        DEBUG_PRINTLN("data dict is empty, not processing");
        return fwInfo;
      }

      JsonObject outerDict = requests[0];

      const char* fwV = nullptr;
      outerDict["firmwareVersion"];
      if (outerDict.containsKey("firmwareVersion")) {
        fwV = outerDict["firmwareVersion"];
      }

      JsonObject innerDict = outerDict["dictonary"];


      if (innerDict.containsKey("firmwareVersion") ) {
        fwV = innerDict["firmwareVersion"];
      }

      String newFWVersion(fwV);

      if ((newFWVersion.indexOf("v") >= 0) || (newFWVersion == NULL)) {
        // newFWVersion = newFWVersion.substring(1);
        newFWVersion = String(FW_REV); // if from outer use the current version as default because we don't update db for the outer json
      }

      fwInfo.fwVersion = newFWVersion.toInt();

      if (innerDict.containsKey("firmwareSize")) {
        const char * fwSize = innerDict["firmwareSize"];
        fwInfo.fileSize = String(fwSize).toInt();
      }

      DEBUG_PRINTF("FW Version Number from cloud response: %s\n", newFWVersion.c_str());
      return fwInfo;
    }

    // fetch ota file and store in spiffs
    size_t fetchOTAFile(const String& remoteFileName, uint32_t expectedFileSize) const
    {
      if (expectedFileSize == 0) {
        DEBUG_PRINTLN("expected file size is zero, not doing anything");
        return 0;
      }
      if (!SPIFFS.begin(true)) {
        DEBUG_PRINTLN("An Error has occurred while mounting SPIFFS, fw upgrades will not work");
      } else {
        DEBUG_PRINTLN("sucessfully mouted spiffs");
      }

      size_t otaFileSize = 0;

      String fullPath = fullyResolvedFilePath(SPIFF_OTA_PATH, remoteFileName);
      DEBUG_PRINTF("Ota file path %s\t\n", fullPath.c_str());
      // check if path exists, return if file is already present
      bool fileExists = SPIFFS.exists(fullPath);
      if (fileExists) {
        DEBUG_PRINTF("file %s already exits, checking integrity\n", fullPath.c_str());
        File file;
        file = SPIFFS.open(fullPath.c_str(), "r");
        if (!file) {
          DEBUG_PRINTF("failed to open file");
          SPIFFS.remove(fullPath);
          return 0 ;
        }
        otaFileSize = file.size();
        DEBUG_PRINTF("Size Of file stored in SPIFFS: %d\n", otaFileSize);
        if ( (otaFileSize == 0) || (otaFileSize != expectedFileSize)) {
          DEBUG_PRINTF("Size Of file stored in SPIFFS invalid, we expected a size of : %d, removing file..\n", expectedFileSize);
          SPIFFS.remove(fullPath);
          return 0 ;
        } else {
          DEBUG_PRINTF("size stored in spiff is equal to exptected : %d\n" , expectedFileSize);
        }
        // TODO:: check with zero or expecte file size
        return otaFileSize;
      }

      // file doesn't exist, procceed to create it
      File f = SPIFFS.open(fullPath.c_str(), FILE_WRITE);

      if (!f) {
        DEBUG_PRINTLN("Failed to open file for writing new version ota file");
        return 0;
      }

      HTTPClient http;
      char urlWithFilename[JSON_MSG_MAX_LEN];
      snprintf(urlWithFilename, JSON_MSG_MAX_LEN, urlOtaSend, remoteFileName.c_str());
      DEBUG_PRINTF("OTA file url %s\t\n", String(urlWithFilename).c_str());
      int connected = http.begin(urlWithFilename);

      if (!connected) {
        DEBUG_PRINTLN("being failed to connect");
      }

      int httpCode = http.GET();
      if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
        otaFileSize = http.writeToStream(&f);
        if ((otaFileSize == 0) || (otaFileSize != expectedFileSize)) {
          DEBUG_PRINTF("incorrect file written, actual size %u, expected size: %u\n", otaFileSize, expectedFileSize);
          f.close();
          SPIFFS.remove(fullPath);
          return 0;
        }
      } else {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        f.close();
        SPIFFS.remove(fullPath);
        return 0;
      }

      DEBUG_PRINTLN("File Written");
      http.end();
      f.close();
      DEBUG_PRINTLN(otaFileSize);
      return otaFileSize;
    }

};

#endif
