#ifndef CLOUDTALK_H
#define CLOUDTALK_H

#include "ArduinoJson.h"
#include <SPIFFS.h>
#include "WiFiOTA.h"
#include "deviceState.h"
#include "hardwareDefs.h"
#include "utils.h"
#include "SensorPayload.h"
#define JSON_MSG_MAX_LEN 512


class CloudTalk {

public:

  /**
    @brief: Create message payload
    @param: Sensor profile of sesnor type
    @return: message payload array
 */

  String createPayload(uint8_t sProfile) {
    char messageCreatePayload[JSON_MSG_MAX_LEN];
    currentLightState();
    switch (sProfile) {
      case SensorProfile::SensorNone:
        DEBUG_PRINTLN("NO Sensor Found");
        break;
      case SensorProfile::SensorTemp:
        PAYLOAD_T.temp = RSTATE.temperature;
        DEBUG_PRINTLN("Creating payload for Temp Sensor");
        snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
                 (macAddrWithoutColons()).c_str(),
                 PAYLOAD_T.temp,
                 PAYLOAD_T.sensorProfile,
                 RSTATE.batteryPercentage);
        DEBUG_PRINTLN(messageCreatePayload);
        break;
      case SensorProfile::SensorTH:
        PAYLOAD_TH.temp = RSTATE.temperature;
        PAYLOAD_TH.humidity = RSTATE.humidity;
        DEBUG_PRINTLN("Creating payload for Temp Humid Sensor");
        snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"humid\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
                 (macAddrWithoutColons()).c_str(),
                 PAYLOAD_TH.temp,
                 PAYLOAD_TH.humidity,
                 PAYLOAD_TH.sensorProfile,
                 RSTATE.batteryPercentage);
        break;
      case SensorProfile::SensorGas:
        DEBUG_PRINTLN("Creating payload for Gas Sensor");
        PAYLOAD_GAS.gas = RSTATE.carbon;
        PAYLOAD_GAS.temp = RSTATE.temperature;
        PAYLOAD_GAS.humidity = RSTATE.humidity;
        snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"gas\":\"%u\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                 (macAddrWithoutColons()).c_str(),
                 PAYLOAD_GAS.gas,
                 PAYLOAD_GAS.temp,
                 PAYLOAD_GAS.humidity,
                 RSTATE.batteryPercentage,
                 PAYLOAD_GAS.sensorProfile);
        DEBUG_PRINTLN(messageCreatePayload);
        break;
      case SensorProfile::SensorLight:
        DEBUG_PRINTLN("Creating payload for Light Control and Analysis");
        PAYLOAD_LIGHT.temp = RSTATE.temperature;
        PAYLOAD_LIGHT.humidity = RSTATE.humidity;
        PAYLOAD_LIGHT.lightState1 = !RSTATE.light_state_1;
        PAYLOAD_LIGHT.lightState2 = !RSTATE.light_state_2;
        PAYLOAD_LIGHT.lightState3 = !RSTATE.light_state_3;
        PAYLOAD_LIGHT.lightState4 = !RSTATE.light_state_4;
        PAYLOAD_LIGHT.lux = RSTATE.lux;
        snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"lux\":\"%.1f\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d,\"light_state_1\":%u,\"light_state_2\":%u,\"light_state_3\":%u,\"light_state_4\":%u}",
                 (macAddrWithoutColons()).c_str(),
                 PAYLOAD_LIGHT.lux,
                 PAYLOAD_LIGHT.temp,
                 PAYLOAD_LIGHT.humidity,
                 RSTATE.batteryPercentage,
                 PAYLOAD_LIGHT.sensorProfile,
                 PAYLOAD_LIGHT.lightState1,
                 PAYLOAD_LIGHT.lightState2,
                 PAYLOAD_LIGHT.lightState3,
                 PAYLOAD_LIGHT.lightState4);
        DEBUG_PRINTLN(messageCreatePayload);
        break;
      default:
        DEBUG_PRINTLN("Not a valid Sensor");
        break;
    }
    return String(messageCreatePayload);
  }

  bool handleSubscribe(char *subscribeString) {
    DEBUG_PRINTLN(subscribeString);
    String clean_subscribeString = String(subscribeString).substring(0, (String(subscribeString).indexOf("}")) + 1);
    DEBUG_PRINTLN(clean_subscribeString.c_str());
    StaticJsonDocument<1024> subscribeJson;
    DeserializationError err = deserializeJson(subscribeJson, clean_subscribeString);

    if (err) {
      DEBUG_PRINTF("deserializeJson() failed : %s\n", err.c_str());
      return false;
    } else {
      DEBUG_PRINTLN("deserializeJson() success");
    }

    DEBUG_PRINTLN("Obtaining action variables");
    int lightState1 = 0;
    int lightState2 = 0;
    int lightState3 = 0;
    int lightState4 = 0;
    if (subscribeJson.containsKey("light_state_1")) {
      lightState1 = subscribeJson["light_state_1"];
    }
    if (subscribeJson.containsKey("light_state_2")) {
      lightState2 = subscribeJson["light_state_2"];
    }
    if (subscribeJson.containsKey("light_state_3")) {
      lightState3 = subscribeJson["light_state_3"];
    }
    if (subscribeJson.containsKey("light_state_4")) {
      lightState4 = subscribeJson["light_state_4"];
    }
    toggleLightState((uint8_t)lightState1, (uint8_t)lightState2, (uint8_t)lightState3, (uint8_t)lightState4);

    return true;
  }


  String createSubscribeTopic(bool ack) {
    char subscribeTopic[128];
    char ackTopic[128];
    if (ack) {
      snprintf(ackTopic, 128, "aws/thing/thinghz/%s/ack", macAddrWithoutColons().c_str());
      return String(ackTopic);
    }
    snprintf(subscribeTopic, 128, "aws/thing/thinghz/%s/light", macAddrWithoutColons().c_str());
    return String(subscribeTopic);
  }
};
#endif
