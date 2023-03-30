#ifndef CLOUDTALKGSM_H
#define CLOUDTALKGSM_H

#include "ArduinoJson.h"
#include <SPIFFS.h>
#include "WiFiOTA.h"
#include "deviceState.h"
#include "hardwareDefs.h"
#include "utils.h"
#include "SensorPayload.h"
#define JSON_MSG_MAX_LEN 512
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>
#include "oledState.h"

const char topic_publish[] = "aws/thing/thinghz/";
const char topic_subscribe[] = "aws/thing/thinghz/light";
const int port = 8883;

class CloudTalkGSM
{

public:
  /**
     @brief: Set MQTT topic to publish
     @return: true when everything works right
  */

  bool setMQTTTopic(TinyGsm *modem)
  {
    modem->sendAT(GF("+CMQTTTOPIC=0,21"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
      return false;
    size_t topic_size = strlen(topic_publish);
    modem->stream.write(topic_publish, topic_size);
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
  }

  /**
     @brief: Creat MQTT payload to publish
     @return: true when everything works right
  */

  bool createMQTTPayload(TinyGsm *modem)
  {
    String payload = createPayload(DEVICE_SENSOR_TYPE);
    size_t payload_size = strlen(payload.c_str());
    modem->sendAT(GF("+CMQTTPAYLOAD=0"), ',', payload_size);
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
      return false;
    modem->stream.write(payload.c_str(), payload_size);
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    return true;
  }

  /**
     @brief: Publish to aws topic
     @return: true when everything works right
  */

  bool publishToTopic(TinyGsm *modem)
  {
    DEBUG_PRINTLN("MQTT publishing message");
    modem->sendAT(GF("+CMQTTPUB=0,1,60"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTPUB: 0,0"));
    if (!res)
    {
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      return false;
    }
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);

    return res;
  }

  /**
     @brief:  Input subscribe topic
     @return: true when everything works right
  */

  bool subscribeToTopic(TinyGsm *modem, int qos)
  {
    size_t topic_size = strlen(topic_subscribe);
    modem->sendAT(GF("+CMQTTSUB=0"), ',', topic_size, ',', qos);
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
      return false;
    modem->stream.write(topic_subscribe, topic_size);
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTSUB: 0,0"));
    if (!res)
    {
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SubscribeFailed);
      return false;
    }
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SubscribeFailed);
    return res;
  }


  /**
      @brief: Configure SSL Configuration
      @return: true when everything goes write
  */

  bool configureSSL(TinyGsm *modem)
  {
    DEBUG_PRINTLN("Configure SSL version");
    modem->sendAT(GF("+CSSLCFG=\"sslversion\",0,4"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;

    DEBUG_PRINTLN("Configure Auth mode");
    modem->sendAT(GF("+CSSLCFG=\"authmode\",0,2"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;

    DEBUG_PRINTLN("Configure SSL cacert");
    modem->sendAT(GF("+CSSLCFG=\"cacert\",0,\"" CACERT_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;

    Serial.println("Configure SSL clientcert");
    modem->sendAT(GF("+CSSLCFG=\"clientcert\",0,\"" CLIENTCERT_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;

    Serial.println("Configure SSL clientkey");
    modem->sendAT(GF("+CSSLCFG=\"clientkey\",0,\"" CLIENTKEY_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;

    return true;
  }

  /**
      @brief: Start MQTT service
      @return: true when everything goes write
  */

  bool startMQTTService(TinyGsm *modem)
  {
    DEBUG_PRINTLN("Starting MQTT");
    modem->sendAT(GF("+CMQTTSTART"));
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTSTART:"));
    if (!res)
    {
      DEBUG_PRINTLN("Could not find +CMQTTSTART");
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    }
    Serial.println("Set SSL Configuration");
    modem->sendAT(GF("+CMQTTSSLCFG=0,0"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    return true;
  }

  /**
      @brief: Accquire MQTT Client
      @return: true when everything goes write
  */
  bool accquireClient(TinyGsm *modem)
  {
    modem->sendAT(GF("+CMQTTACCQ=0,\"" MQTT_CLIENT_NAME "\",1"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
      return false;
    return true;
  }

  /**
      @brief: make MQTT Connection
      @return: true when everything goes write
  */
  bool connectMQTT(TinyGsm *modem)
  {
    modem->sendAT(GF("+CMQTTCONNECT=0,\"" MQTT_HOST "\",90,1"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTCONNECT: 0,0"));
    if (!res)
    {
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
    }
    else
    {
      clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
    }
    return res;
  }

  /**
      @brief: release MQTT Client
      @return: true when everything goes write
  */
  bool releaseMQTTClient(TinyGsm *modem)
  {
    DEBUG_PRINTLN("RElease Mqtt Client");
    modem->sendAT(GF("+CMQTTREL?"));
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
  }

  /**
      @brief: Open GSN Network
      @return: true when everything goes write
  */

  bool openNetwork(TinyGsm *modem)
  {
    modem->sendAT(GF("+NETOPEN"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, ("+NETOPEN: 0")) != 1)
      return false;
    return true;
  }

  /**
      @brief: Stop mqtt service and stop connection
      @return: true when everything goes write
  */
  bool stopMQTTClient(TinyGsm *modem)
  {
    DEBUG_PRINTLN("Stop Mqtt Client");
    modem->sendAT(GF("+CMQTTSTOP"));
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
  }

  /**
      @brief: disconnect mqtt connection
      @param: TinyGsm pointer
      @return: true when everything goes write
  */
  bool disconnectMQTTClient(TinyGsm *modem)
  {
    DEBUG_PRINTLN("Stop Mqtt Client");
    modem->sendAT(GF("+CMQTTDISC=0,120"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTDISC: 0,0"));
    return res;
  }

  /**
        @brief: Update NTP Time
        @param: Pointer to TinyGSM
        @return: message payload array
  */
  bool updateNTPTime(TinyGsm *modem)
  {
    DEBUG_PRINTLN("Update NTP Time");
    modem->sendAT("+CNTP=\"" NTP_SERVER "\",32");
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
  }

  /**
    @brief: Create message payload
    @param: Sensor profile of sesnor type
    @return: message payload array
 */

  String createPayload(uint8_t sProfile)
  {
    char messageCreatePayload[JSON_MSG_MAX_LEN];
    currentLightState();
    switch (sProfile)
    {
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
      break;
    case SensorProfile::SensorLight:
      DEBUG_PRINTLN("Creating payload for Light Control and Analysis");
      PAYLOAD_LIGHT.temp = RSTATE.temperature;
      PAYLOAD_LIGHT.humidity = RSTATE.humidity;
      PAYLOAD_LIGHT.lightState1 = !RSTATE.light_state_1;
      PAYLOAD_LIGHT.lightState2 = !RSTATE.light_state_2;
      PAYLOAD_LIGHT.lux = RSTATE.lux;
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"lux\":\"%.1f\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d,\"light_state_1\":%u,\"light_state_2\":%u}",
               (macAddrWithoutColons()).c_str(),
               PAYLOAD_LIGHT.lux,
               PAYLOAD_LIGHT.temp,
               PAYLOAD_LIGHT.humidity,
               RSTATE.batteryPercentage,
               PAYLOAD_LIGHT.sensorProfile,
               PAYLOAD_LIGHT.lightState1,
               PAYLOAD_LIGHT.lightState2);
      break;
    default:
      DEBUG_PRINTLN("Not a valid Sensor");
      break;
    }
    return String(messageCreatePayload);
  }

  /**
        @brief: restart Modem
        @param: TinyGsm pointer
        @return: true when everything goes write
  */

  bool restartModem(TinyGsm *modem)
  {
    int ret = modem->restart();
    String modemInfo = modem->getModemInfo();
    DEBUG_PRINTF("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
    if (modem->getSimStatus() == 0)
    {
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    else
    {
      clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    return ret;
  }

  /**
        @brief: Initialise Modem
        @param: TinyGsm pointer
        @return: true when everything goes write
  */

  bool initialiseModem(TinyGsm *modem)
  {
    DEBUG_PRINTLN("initialising Modem");
    int ret = modem->init();
    String modemInfo = modem->getModemInfo();
    DEBUG_PRINTF("modemInfo: %s\n SimStatus: %d\n", modemInfo.c_str(), modem->getSimStatus());
    if (modem->getSimStatus() == 0)
    {
      setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    else
    {
      clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    return ret;
  }

  bool handleSubscribe(char *subscribeString)
  {
    RSTATE.displayEvents = DisplayEventActionReceived;
    String clean_subscribeString = String(subscribeString).substring(0,(String(subscribeString).indexOf("}"))+1);
    DEBUG_PRINTLN(clean_subscribeString.c_str()); 
    drawDisplay(RSTATE.displayEvents);
    StaticJsonDocument<1024> subscribeJson;
    DeserializationError err = deserializeJson(subscribeJson, subscribeString);

    if (err)
    {
      DEBUG_PRINTF("deserializeJson() failed : %s\n", err.c_str());
      return false;
    }
    else
    {
      DEBUG_PRINTLN("deserializeJson() success");
    }

    DEBUG_PRINTLN("Obtaining action variables");
    int lightState1=0;
    int lightState2=0;
    if (subscribeJson.containsKey("light_state_1")) {
         lightState1 = subscribeJson["light_state_1"];
         
      }
    if (subscribeJson.containsKey("light_state_2")) {
         lightState2 = subscribeJson["light_state_2"];
      }
    toggleLightState((uint8_t)lightState1, (uint8_t)lightState2);   
    if (subscribeJson.containsKey("light_thresh"))
        RSTATE.light_thresh = subscribeJson["light_thresh"];
    DEBUG_PRINTF("light intesity is %d",RSTATE.light_thresh);
    
    return true; 
  }


  String createSubscribeTopic(bool ack){
      char subscribeTopic[128];
      char ackTopic[128];
      if(ack){
        snprintf(ackTopic, 128, "aws/thing/thinghz/%s/ack",macAddrWithoutColons().c_str());
        return String(ackTopic);  
      }
      snprintf(subscribeTopic, 128, "aws/thing/thinghz/%s/light",macAddrWithoutColons().c_str());
      return String(subscribeTopic);
  }

};

#endif
