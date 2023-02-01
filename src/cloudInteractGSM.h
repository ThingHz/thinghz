#ifndef CLOUDTALKGSM_H
#define CLOUDTALKGSM_H

#include "ArduinoHttpClient.h"

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

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

const char apn[] = "airtelgprs.com";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char host[] = "d1nt4kt9rfh6o9.cloudfront.net";
const char resource[] = "/sim";
const int port = 443;

class CloudTalkGSM
{
public:
    /**
       @brief: Create GSM http link and send the device payload to cloud
       @return: true when everything works right
    */

    bool sendPayload()
    {
        String retJson;
        int httpCode;
        HttpClient http(client, host, port);
        if (!modem.waitForNetwork())
        {
            DEBUG_PRINTLN("Failed to connect to network, Restarting modem");
            modemRestart();
            return false;
        }
        if (modem.isNetworkConnected())
        {
            DEBUG_PRINTLN("Network Connected");
        }
        if (!modem.gprsConnect(apn, gprsUser, gprsPass))
        {
            DEBUG_PRINTLN("Failed to connect to GPRS, Reseting modem");
            restartModem();
            return false;
        }
        if (modem.isGprsConnected())
        {
            DEBUG_PRINTLN("GPRS connected");
        }

        String messagePayload = createPayload(DEVICE_SENSOR_TYPE);
        DEBUG_PRINTLN(messagePayload);
        http.connectionKeepAlive();
        http.beginRequest();
        http.post(resource);
        http.sendHeader("Content-Type", "application/json");
        http.sendHeader("Content-Length", String(messagePayload.length()));
        http.beginBody();
        http.print(messagePayload);
        http.endRequest();
        retJson = http.responseBody();
        DEBUG_PRINTLN(retJson);
        if (http.responseStatusCode() == 200)
        {
            retJson = http.responseBody();
            DEBUG_PRINTLN(retJson);
        }
        else
        {
            DEBUG_PRINTF("[HTTP] POST... failed , code is %d\n",  http.responseStatusCode());
            return false;
        }
       
        yield();

        http.stop();

        return true;
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

    void restartModem()
    {
        modem.restart();
        String modemInfo = modem.getModemInfo();
        DEBUG_PRINTF("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem.getSimStatus());
        if (modem.getSimStatus() == 0)
        {
            setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
        }
        else
        {
            clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
        }
    }

    void initialiseModem()
    {
        modem.init();
        String modemInfo = modem.getModemInfo();
        DEBUG_PRINTF("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem.getSimStatus());
        if (modem.getSimStatus() == 0)
        {
            setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
        }
        else
        {
            clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
        }
    }
};

#endif
