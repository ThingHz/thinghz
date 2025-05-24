#ifndef SENSOR_PAYLOAD_H
#define SENSOR_PAYLOAD_H

#include "hardwareDefs.h"
#include "assert.h"
#include "deviceState.h"
#include "WiFiOTA.h"

enum SensorProfile {
    SensorNone = 0,
    SensorTemp,
    SensorTH,
    SensorGas,
    SensorLight
};

enum HardewareRevision{
    Thinghz_Plant_IQ = 1,
    Thinghz_Farmsense,
    Thinghz_Indoor
};

enum SoftwareRevision{
    Thinghz_Farmsense_Node = 1,
    Thinghz_Farmsense_Stanalone,
    Thinghz_Farmsense_PlantIQ_T,
    Thinghz_Farmsense_PlantIQ_TH,
    Thinghz_Farmsense_PlantIQ_THL,
};

const char* sensorProfileToString(uint8_t sProfile);


typedef struct SensorPayloadT{
    float temp;
} T_SensorPayloadT;

typedef struct SensorPayloadTH{
    T_SensorPayloadT* sensorPayloadT;
    float humidity;
} T_SensorPayloadTH;

typedef struct SensorPayloadGas{
    T_SensorPayloadTH* sensorPayloadTH;
    uint16_t gas;
} T_SensorPayloadGas;

typedef struct SensorPayloadLight{
    T_SensorPayloadTH* sensorPayloadTH;
    float lux;
    uint8_t lightState1;
    uint8_t lightState2;
    uint8_t lightState3;
    uint8_t lightState4;
}T_SensorPayloadLight;

typedef struct SensorPayload{
    uint8_t sensorProfile;
    uint8_t hwRev;
    uint8_t fwRev;
    uint8_t batteryPercentage;
    T_SensorPayloadTH* t_sensorTH;
    T_SensorPayloadLight* t_sensorLight;
    T_SensorPayloadGas* t_sensorGas;
} T_SensorPayload;


char* createSensorTpayload(T_SensorPayload*, char*, size_t);
char* createSensorTHpayload(T_SensorPayload*, char*, size_t);
char* createSensorGasPayload(T_SensorPayload*, char*, size_t);
char* createSensorLightPayload(T_SensorPayload*, char*, size_t);

char* (*fp_CreatePayload)[4](T_SensorPayload*, char*, size_t) = {createSensorTpayload, createSensorTHpayload, createSensorGasPayload, createSensorLightPayload};


#endif // SENSOR_PAYLOAD_H
