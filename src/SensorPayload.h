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
  SensorGyroAccel,
  SensorTHM,
  SensorTHC,
  SensorBMP,
  SensorBMPTH
};



const char* sensorProfileToString(uint8_t sProfile) {
  //Here After and before const means that you can not change pointer as well as data
  static const char* const map[] = {
    "SensorNone", "SensorTemp", "SensorTH", "SensorGas"
    "SensorGyroAccel", "SensorTHM","SensorTHC","SensorBMP","SensorBMPTH"
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
    sensorProfile = SensorProfile::SensorGyroAccel;
  }

  int16_t gyro;
  int16_t accel;
} __attribute__ ((packed));

struct SensorPayloadTHC : public SensorPayloadTH {
  SensorPayloadTHC(): capcitance(INVALID_CAP_READING) {
    sensorProfile = SensorProfile::SensorTHC;
  }

  float capcitance;
} __attribute__ ((packed));


SensorPayload           sensorPayload;
SensorPayloadT          sensorPayloadT;
SensorPayloadTH         sensorPayloadTH;
SensorPayloadGyroAccel  sensorPayloadGyroAccel;
SensorPayloadTHM        sensorPayloadTHM;
SensorPayloadGas        sensorPayloadGas;
SensorPayloadTHC        sensorPayloadTHC;


#define PAYLOAD_NONE   sensorPayload
#define PAYLOAD_T      sensorPayloadT
#define PAYLOAD_TH     sensorPayloadTH
#define PAYLOAD_THM    sensorPayloadTHM
#define PAYLOAD_GA     sensorPayloadGyroAccel
#define PAYLOAD_GAS    sensorPayloadGas
#define PAYLOAD_THC    sensorPayloadTHC

#endif
