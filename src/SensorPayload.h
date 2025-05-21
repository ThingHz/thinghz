#ifndef SENSOR_PAYLOAD_H
#define SENSOR_PAYLOAD_H

#include "hardwareDefs.h"
#include "assert.h"
#include "deviceState.h"

enum SensorProfile {
    SensorNone = 1,
    SensorTemp,
    SensorTH,
    SensorGas,
    SensorLight
};

const char* sensorProfileToString(uint8_t sProfile);

struct SensorPayload {
public:
    SensorPayload();
    uint8_t sensorProfile;
    uint8_t hwRev;
    uint8_t fwRev;
    uint8_t batteryPercentage;
} __attribute__((packed));

struct SensorPayloadT : public SensorPayload {
    SensorPayloadT();
    float temp;
} __attribute__((packed));

struct SensorPayloadTH : public SensorPayloadT {
    SensorPayloadTH();
    float humidity;
} __attribute__((packed));

struct SensorPayloadGas : public SensorPayloadTH {
    SensorPayloadGas();
    uint16_t gas;
} __attribute__((packed));

struct SensorPayloadLight : public SensorPayloadTH {
    SensorPayloadLight();
    float lux;
    uint8_t lightState1;
    uint8_t lightState2;
    uint8_t lightState3;
    uint8_t lightState4;
} __attribute__((packed));

extern SensorPayload sensorPayload;
extern SensorPayloadT sensorPayloadT;
extern SensorPayloadTH sensorPayloadTH;
extern SensorPayloadGas sensorPayloadGas;
extern SensorPayloadLight sensorPayloadLight;

#define PAYLOAD_NONE sensorPayload
#define PAYLOAD_T sensorPayloadT
#define PAYLOAD_TH sensorPayloadTH
#define PAYLOAD_GAS sensorPayloadGas
#define PAYLOAD_LIGHT sensorPayloadLight

#endif // SENSOR_PAYLOAD_H
