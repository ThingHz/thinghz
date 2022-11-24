#ifndef CLOUDTALK_H
#define CLOUDTALK_H

#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <SPIFFS.h>
#include "WiFiOTA.h"
#include "deviceState.h"
#include "hardwareDefs.h"
#include "utils.h"
#include "SensorPayload.h"
#define JSON_MSG_MAX_LEN          512

DynamicJsonDocument jsonDocument(1024);


//const char urlmessageSend[] = "https://ir989t4sy0.execute-api.us-east-1.amazonaws.com/prod/data";
const char urlmessageSend[] = "https://api.thinghz.com/v1/data";
const char urlOtaSend[]     = "https://api.thinghz.com/v1/data/download-file?filename=%s"; //URL for ota file download

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
       @brief: iterate through the linked list and sends data to cloud
       @param: deviceState object
       @return: true when data sent is successful
    */
    bool sendSensorData(DeviceState &devState){
        String retJson;
        SensorPayload *payload = nullptr;
        while (devState.hasUnprocessedTelemetry()){
          DEBUG_PRINTLN("hasUnprocessedTelemetry");
          PayloadQueueElement *telemeteryElement = devState.telemetryQueue.pop();
          payload = telemeteryElement->_payload;
          DEBUG_PRINTF("processing data for Sensor payload: deviceType: %u , sensor profile: %u, hwRev: %u and fwRev:%u\n",
                     payload->deviceType, payload->sensorProfile, payload->hwRev, payload->fwRev);
          String preparedPayload;
          preparedPayload = _createPayload(payload->sensorProfile,telemeteryElement);
          bool dataSent = false;
          retJson = _sendPayload(preparedPayload,urlmessageSend);
          DEBUG_PRINTLN(retJson);
          if (retJson == NULL)
        {
          DEBUG_PRINTLN("http send returned nothing...");
          rtcState.missedDataPointsCounter++;
          //dataLog.append(preparedPayload.c_str(),false);
          return false;
        
        }
         delete telemeteryElement;
      }
       
        return true;
    }


  private:  
    /**
       @brief: Create http link and send the device payload to cloud
       @param: payload and url to send
       @return: true when everything works right
    */
    String _sendPayload(const String &Payload, const char *url)
    {

      String retJson;
      int httpCode;
      HTTPClient http;
      http.begin(url);//zx     //test
      http.addHeader("Content-Type" , "application/json");  //Specify content-type header
      DEBUG_PRINTLN(Payload);
      httpCode = http.POST(Payload);     //Send the request
      yield();
      if (httpCode == HTTP_CODE_OK && httpCode > 0) {
        retJson = http.getString();   //Get the response payload
        DEBUG_PRINTLN(retJson);
      } else {
        DEBUG_PRINTF("[HTTP] GET... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
      }
      http.end();
      return retJson;
    }

 /**
       @brief: Create message payload
       @param: Sensor profile of sesnor type
       @return: message payload array
    */

    String _createPayload(uint8_t sProfile,PayloadQueueElement *telemeteryElement) {
      char messageCreatePayload[JSON_MSG_MAX_LEN];
      switch (sProfile) {
        case SensorProfile::SensorNone :
          DEBUG_PRINTLN("NO Sensor Found");
          break;
        case SensorProfile::SensorTemp :
          {
          SensorPayloadTemp *payload = (SensorPayloadTemp *) telemeteryElement->_payload;
          String macStr(telemeteryElement->_mac);
          DEBUG_PRINTLN("Creating payload for Temp Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"sensor_profile\": %d,\"bat\": \"%d\"}",
                   macStr.c_str(),
                   payload->temperature,
                   payload->sensorProfile,
                   payload->batteryPercentage                  );
          DEBUG_PRINTLN(messageCreatePayload);
          }
          break;
        case SensorProfile::SensorTH :
          {
          SensorPayloadTH *payload = (SensorPayloadTH *) telemeteryElement->_payload;
          String macStr(telemeteryElement->_mac);
          DEBUG_PRINTLN("Creating payload for Temp Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"humid\":\"%.1f\",\"sensor_profile\": %d,\"bat\": \"%d\"}",
                   macStr.c_str(),
                   payload->temperature,
                   payload->humidity,
                   payload->sensorProfile,
                   payload->batteryPercentage                  );
          DEBUG_PRINTLN(messageCreatePayload);
          }
          break;
        case SensorProfile::SensorGas :
          {
            SensorPayloadTHCO2 *payload = (SensorPayloadTHCO2 *) telemeteryElement->_payload;
          String macStr(telemeteryElement->_mac);
          DEBUG_PRINTLN("Creating payload for Temp Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"humid\":\"%.1f\",\"gas\":\"%u\",\"sensor_profile\": %d,\"bat\": \"%d\"}",
                   macStr.c_str(),
                   payload->temperature,
                   payload->humidity,
                   payload->co2ppm,
                   payload->sensorProfile,
                   payload->batteryPercentage                  );
          DEBUG_PRINTLN(messageCreatePayload);
          }
          break;
        default:
          DEBUG_PRINTLN("Not a valid Sensor");
          delete telemeteryElement;
          break;
      }
      return String(messageCreatePayload);
    }

};

#endif
