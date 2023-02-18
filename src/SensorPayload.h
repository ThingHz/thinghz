#ifndef SENSOR_H
#define SENSOR_H

#include "hardwareDefs.h"
#include "assert.h"
#include "deviceState.h"


enum SensorProfile {
  SensorNone  = 1,
  SensorTemp,
  SensorTH,
  SensorGas,
  SensorLight
};



const char* sensorProfileToString(uint8_t sProfile) {
  //Here After and before const means that you can not change pointer as well as data
  static const char* const map[] = {
    "SensorNone", "SensorTemp", "SensorTH", "SensorGas", "SensorLight"
  };
  return map[sProfile];
}

struct SensorPayload {
  public:
    SensorPayload(): sensorProfile(SensorProfile::SensorNone), hwRev(HW_REV), fwRev(FW_REV), batteryPercentage(BATTERY_INITIAL_READING) {
    }

    uint8_t sensorProfile;
    uint8_t hwRev;
    uint8_t fwRev;
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


struct SensorPayloadGas : public SensorPayloadTH {
  SensorPayloadGas(): gas(INVALID_GAS_READING) {
    sensorProfile = SensorProfile::SensorGas;
  }

  uint16_t gas;
} __attribute__ ((packed));

struct SensorPayloadLight : public SensorPayloadTH {
  SensorPayloadLight(): 
  lux(INVALID_GAS_READING),
  lightState(DEFAULT_STATE_READING) {
    sensorProfile = SensorProfile::SensorLight;
  }

  float lux;
  uint8_t  lightState;
} __attribute__ ((packed));


SensorPayload           sensorPayload;
SensorPayloadT          sensorPayloadT;
SensorPayloadTH         sensorPayloadTH;
SensorPayloadGas        sensorPayloadGas;
SensorPayloadLight      sensorPayloadLight;

#define PAYLOAD_NONE    sensorPayload
#define PAYLOAD_T       sensorPayloadT
#define PAYLOAD_TH      sensorPayloadTH
#define PAYLOAD_GAS     sensorPayloadGas
#define PAYLOAD_LIGHT   sensorPayloadLight

#endif
