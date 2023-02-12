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

const char topic_publish[] = "aws/thing/thing_test/";
const char topic_subscribe[] = "aws/thing/thing_test/light";
const int port = 8883;
  


class CloudTalkGSM
{
  
  public:
    /**
       @brief: Set MQTT topic to publish
       @return: true when everything works right
    */

    bool setMQTTTopic(TinyGsm *modem) {
      modem->sendAT(GF("+CMQTTTOPIC=0,21"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
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
    
    bool createMQTTPayload(TinyGsm *modem){
      String payload = createPayload(DEVICE_SENSOR_TYPE);
      size_t payload_size = strlen(payload.c_str());
      modem->sendAT(GF("+CMQTTPAYLOAD=0"),',',payload_size);
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
      modem->stream.write(payload.c_str(),payload_size);
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
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTPUB: 0,0"));
      if (!res){
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

    bool subscribeToTopic(TinyGsm *modem,int qos) {
      size_t topic_size = strlen(topic_subscribe);
      modem->sendAT(GF("+CMQTTSUB=0"),',',topic_size,',',qos);
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
      modem->stream.write(topic_subscribe, topic_size);
      modem->stream.write(GSM_NL);
      modem->stream.flush();
      modem->waitResponse(AT_WAIT_TIME_MSECS);
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTSUB: 0,0"));
      if (!res){
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

    bool configureSSL(TinyGsm *modem) {
      DEBUG_PRINTLN("Configure SSL version");
      modem->sendAT(GF("+CSSLCFG=\"sslversion\",0,4"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
      
      DEBUG_PRINTLN("Configure Auth mode");
      modem->sendAT(GF("+CSSLCFG=\"authmode\",0,2"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
  
      DEBUG_PRINTLN("Configure SSL cacert");
      modem->sendAT(GF("+CSSLCFG=\"cacert\",0,\"" CACERT_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
     
      Serial.println("Configure SSL clientcert");
      modem->sendAT(GF("+CSSLCFG=\"clientcert\",0,\"" CLIENTCERT_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
  
      Serial.println("Configure SSL clientkey");
      modem->sendAT(GF("+CSSLCFG=\"clientkey\",0,\"" CLIENTKEY_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;

      return true;
    }


    /**
        @brief: Start MQTT service
        @return: true when everything goes write
    */

   bool startMQTTService(TinyGsm *modem){
    DEBUG_PRINTLN("Starting MQTT");
    modem->sendAT(GF ("+CMQTTSTART"));
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTSTART:"));
    if(!res){
        DEBUG_PRINTLN("Could not find +CMQTTSTART");
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
      }
    Serial.println("Set SSL Configuration");
    modem->sendAT(GF ("+CMQTTSSLCFG=0,0"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    return true;
   }

    /**
        @brief: Accquire MQTT Client
        @return: true when everything goes write
    */
   bool accquireClient(TinyGsm *modem){
     modem->sendAT(GF("+CMQTTACCQ=0,\"" MQTT_CLIENT_NAME "\",1"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
    return true;
   }

    /**
        @brief: make MQTT Connection
        @return: true when everything goes write
    */
    bool connectMQTT(TinyGsm *modem) {
      modem->sendAT(GF("+CMQTTCONNECT=0,\"" MQTT_HOST "\",90,1"));
      modem->waitResponse();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTCONNECT: 0,0"));
      if (!res){
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      }else{
        clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      }
      return res;
    }

    /**
        @brief: release MQTT Client
        @return: true when everything goes write
    */
    bool releaseMQTTClient(TinyGsm *modem) {
      DEBUG_PRINTLN("RElease Mqtt Client");
      modem->sendAT(GF("+CMQTTREL?"));
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
      return res;
    }

    /**
        @brief: Open GSN Network
        @return: true when everything goes write
    */
    
    bool openNetwork(TinyGsm *modem){
        modem->sendAT(GF("+NETOPEN"));
        if (modem->waitResponse(AT_WAIT_TIME_MSECS, ("+NETOPEN: 0")) != 1) return false;
        return true;
    }

    /**
        @brief: Stop mqtt service and stop connection
        @return: true when everything goes write
    */
    bool stopMQTTClient(TinyGsm *modem) {
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
    bool disconnectMQTTClient(TinyGsm *modem) {
      DEBUG_PRINTLN("Stop Mqtt Client");
      modem->sendAT(GF("+CMQTTDISC=0,120"));
      modem->waitResponse();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTDISC: 0,0"));
      return res;
    }


    /**
          @brief: Create message payload
          @param: Sensor profile of sesnor type
          @return: message payload array
    */
   bool updateNTPTime(TinyGsm *modem){
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
      switch (sProfile)
      {
        case SensorProfile::SensorNone:
          DEBUG_PRINTLN("NO Sensor Found");
          break;
        case SensorProfile::SensorTemp:
          PAYLOAD_T.temp = RSTATE.temperature;
          DEBUG_PRINTLN("Creating payload for Temp Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
                   (PSTATE.deviceId).c_str(),
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
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_TH.temp,
                   PAYLOAD_TH.humidity,
                   PAYLOAD_TH.sensorProfile,
                   RSTATE.batteryPercentage);
          break;
        case SensorProfile::SensorTHM:
          DEBUG_PRINTLN("Creating payload for Temp Humid Moist Sensor");
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"moisture\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_THM.temp,
                   PAYLOAD_THM.humidity,
                   PAYLOAD_THM.moisture,
                   RSTATE.batteryPercentage,
                   PAYLOAD_TH.sensorProfile);

          break;
        case SensorProfile::SensorGas:
          DEBUG_PRINTLN("Creating payload for Gas Sensor");
          PAYLOAD_GAS.gas = RSTATE.carbon;
          PAYLOAD_GAS.temp = RSTATE.temperature;
          PAYLOAD_GAS.humidity = RSTATE.humidity;
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"gas\":\"%u\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_GAS.gas,
                   PAYLOAD_GAS.temp,
                   PAYLOAD_GAS.humidity,
                   RSTATE.batteryPercentage,
                   PAYLOAD_GAS.sensorProfile);
          break;
        case SensorProfile::SensorGyroAccel:
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
        case SensorProfile::SensorBMP:
          DEBUG_PRINTLN("Creating payload for BMP Sensor");
          PAYLOAD_BMP.bmp_temp = RSTATE.bmpTemp;
          PAYLOAD_BMP.bmp_altitude = RSTATE.altitude;
          PAYLOAD_BMP.bmp_pressure = RSTATE.bmphPa;
          PAYLOAD_BMP.bmp_sea = RSTATE.seaLevel;
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"bmpTemp\":\"%.1f\",\"bmpAltitude\":\"%.1f\",\"bmpPressue\":\"%.1f\",\"bmpSea\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_BMP.bmp_temp,
                   PAYLOAD_BMP.bmp_altitude,
                   PAYLOAD_BMP.bmp_pressure,
                   PAYLOAD_BMP.bmp_sea,
                   RSTATE.batteryPercentage,
                   PAYLOAD_BMP.sensorProfile);
          break;
        case SensorProfile::SensorBMPTH:
          DEBUG_PRINTLN("Creating payload for BMP Sensor");
          PAYLOAD_TH_BMP.bmp_temp = RSTATE.bmpTemp;
          PAYLOAD_TH_BMP.bmp_humid = RSTATE.humidity;
          PAYLOAD_TH_BMP.bmp_altitude = RSTATE.altitude;
          PAYLOAD_TH_BMP.bmp_pressure = RSTATE.bmphPa;
          PAYLOAD_TH_BMP.bmp_sea = RSTATE.seaLevel;
          snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"device_id\":\"%s\",\"bmpTemp\":\"%.1f\",\"humid\":\"%.1f\",\"bmpAltitude\":\"%.1f\",\"bmpPressue\":\"%.1f\",\"bmpSea\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
                   (PSTATE.deviceId).c_str(),
                   PAYLOAD_TH_BMP.bmp_temp,
                   PAYLOAD_TH_BMP.bmp_humid,
                   PAYLOAD_TH_BMP.bmp_altitude,
                   PAYLOAD_TH_BMP.bmp_pressure,
                   PAYLOAD_TH_BMP.bmp_sea,
                   RSTATE.batteryPercentage,
                   PAYLOAD_TH_BMP.sensorProfile);
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
    { DEBUG_PRINTLN("initialising Modem");
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
    
};

#endif
