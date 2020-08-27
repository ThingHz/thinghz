#ifndef SENSOR_H
#define SENSOR_H

#include "hardwareDefs.h"
#include "assert.h"
#define JSON_MSG_MAX_LEN                        512


enum SensorProfile {
  SensorNone  = 1,
  SensorTemp,
  SensorTH,
  SensorGas,
  SensorGyroAccel,
  SensorTHM
};



const char* sensorProfileToString(uint8_t sProfile) {
  //Here After and before const means that you can not change pointer as well as data
  static const char* const map[] = {
    "SensorNone", "SensorTemp", "SensorTH", "SensorGas"
    "SensorGyroAccel", "SensorTHM"
  };
  return map[sProfile];
}

struct SensorPayload {
  public:
    SensorPayload(): sensorProfile(SensorProfile::SensorNone), hwRev(HW_REV), fwRev(FW_REV), deviceType(DEVICE_TYPE),
      batteryPercentage(BATTERY_INITIAL_READING) {
    }

    uint8_t sensorProfile;
    uint8_t hwRev;
    uint8_t fwRev;
    uint8_t deviceType;
    uint8_t batteryPercentage;
} __attribute__ ((packed));

//Inherit properties of Sensor payload in other Sensor structure accoarding to Sensor Profile

struct SensorPayloadT : public SensorPayload {
  SensorPayloadT(): temp(INVALID_TEMP_READING) {
    sensorProfile = SensorProfile::SensorTemp;
  }

  float temp;
} __attribute__ ((packed));

struct SensorPayloadTH : public SensorPayloadT {
  SensorPayloadTH(): humidity(INVALID_HUMIDITY_READING) {
    sensorProfile = SensorProfile::SensorTH;
  }

  float humidity;
} __attribute__ ((packed));

struct SensorPayloadTHM : public SensorPayloadTH {
  SensorPayloadTHM(): moisture(INVALID_MOISTURE_READING) {
    sensorProfile = SensorProfile::SensorTHM;
  }

  float moisture;
} __attribute__ ((packed));

struct SensorPayloadGas : public SensorPayload {
  SensorPayloadGas(): gas(INVALID_GAS_READING) {
    sensorProfile = SensorProfile::SensorGas;
  }

  uint16_t gas;
} __attribute__ ((packed));

struct SensorPayloadGyroAccel : public SensorPayload {
  SensorPayloadGyroAccel(): gyro(INVALID_GYRO_READING), accel(INVALID_ACCEL_READING) {
    sensorProfile = SensorProfile::SensorTH;
  }

  int16_t gyro;
  int16_t accel;
} __attribute__ ((packed));

SensorPayload           sensorPayload;
SensorPayloadT          sensorPayloadT;
SensorPayloadTH         sensorPayloadTH;
SensorPayloadGyroAccel  sensorPayloadGyroAccel;
SensorPayloadTHM        sensorPayloadTHM;
SensorPayloadGas        sensorPayloadGas;




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
      sensorPayloadT.temp = RSTATE.temperature;
      DEBUG_PRINTLN("Creating payload for Temp Sensor");
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"Item\":{\"temp\": \"%.1f\",\"profile\": %d,\"battery\": \"%d\"}}",
               sensorPayloadT.temp,
               sensorPayloadT.sensorProfile,
               RSTATE.batteryPercentage
              );
      DEBUG_PRINTLN(messageCreatePayload);
      break;
    case SensorProfile::SensorTH :
      sensorPayloadTH.temp = RSTATE.temperature;
      sensorPayloadTH.humidity = RSTATE.humidity;
      DEBUG_PRINTLN("Creating payload for Temp Humid Sensor");
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"Item\":{\"temp\": \"%.1f\",\"humid\": \"%.1f\",\"profile\": %d,\"battery\": \"%d\"}}",
               sensorPayloadTH.temp,
               sensorPayloadTH.humidity,
               sensorPayloadTH.sensorProfile,
               RSTATE.batteryPercentage
              );
      break;
    case SensorProfile::SensorTHM :
      DEBUG_PRINTLN("Creating payload for Temp Humid Moist Sensor");
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"deviceId\":\"%s\",\"temperature\":\"%.1f\",\"humidity\":\"%.1f\",\"moisture\":\"%.1f\",\"batteryPercentage\":\"%d\",\"sensorProfile\":%d}",
               (RSTATE.macAddr).c_str(),
               sensorPayloadTHM.temp,
               sensorPayloadTHM.humidity,
               sensorPayloadTHM.moisture,
               RSTATE.batteryPercentage,
               sensorPayloadTHM.sensorProfile);

      break;
    case SensorProfile::SensorGas :
      DEBUG_PRINTLN("Creating payload for Gas Sensor");
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"deviceId\":\"%s\",\"gas\":\"%u\",\"batteryPercentage\":\"%d\",\"sensorProfile\":%d}",
               (RSTATE.macAddr).c_str(),
               sensorPayloadGas.gas,
               RSTATE.batteryPercentage,
               sensorPayloadGas.sensorProfile);
      break;
    case SensorProfile::SensorGyroAccel :
      DEBUG_PRINTLN("Creating payload for Temp Humid Sensor");
      snprintf(messageCreatePayload, JSON_MSG_MAX_LEN, "{\"deviceId\":\"%s\",\"gyro\":\"%d\",\"accel\":\"%d\",\"batteryPercentage\":\"%d\",\"sensorProfile\":%d}",
               (RSTATE.macAddr).c_str(),
               sensorPayloadGyroAccel.gyro,
               sensorPayloadGyroAccel.accel,
               RSTATE.batteryPercentage,
               sensorPayloadGyroAccel.sensorProfile);
      break;
    default:
      DEBUG_PRINTLN("Not a valid Sensor");
      break;
  }
  return String(messageCreatePayload);
}

#define PAYLOAD_NONE   sensorPayload
#define PAYLOAD_T      sensorPayloadT
#define PAYLOAD_TH     sensorPayloadTH
#define PAYLOAD_THM    sensorPayloadTHM
#define PAYLOAD_GA     sensorPayloadGyroAccel
#define PAYLOAD_GAS    sensorPayloadGas

#endif
