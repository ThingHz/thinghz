#include "SensorPayload.h"

SensorPayload sensorPayload;
SensorPayloadT sensorPayloadT;
SensorPayloadTH sensorPayloadTH;
SensorPayloadGas sensorPayloadGas;
SensorPayloadLight sensorPayloadLight;

const char* sensorProfileToString(uint8_t sProfile) {
    static const char* const map[] = {
        "SensorNone", "SensorTemp", "SensorTH", "SensorGas", "SensorLight"
    };
    return map[sProfile];
}

SensorPayload::SensorPayload()
    : sensorProfile(SensorProfile::SensorNone), hwRev(HW_REV), fwRev(FW_REV), batteryPercentage(BATTERY_INITIAL_READING) {
}

SensorPayloadT::SensorPayloadT()
    : temp(INVALID_TEMP_READING) {
    sensorProfile = SensorProfile::SensorTemp;
}

SensorPayloadTH::SensorPayloadTH()
    : humidity(INVALID_HUMIDITY_READING) {
    sensorProfile = SensorProfile::SensorTH;
}

SensorPayloadGas::SensorPayloadGas()
    : gas(INVALID_GAS_READING) {
    sensorProfile = SensorProfile::SensorGas;
}

SensorPayloadLight::SensorPayloadLight()
    : lux(INVALID_GAS_READING),
      lightState1(DEFAULT_STATE_READING),
      lightState2(DEFAULT_STATE_READING),
      lightState3(DEFAULT_STATE_READING),
      lightState4(DEFAULT_STATE_READING) {
    sensorProfile = SensorProfile::SensorLight;
}
