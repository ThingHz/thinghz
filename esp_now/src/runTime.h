#ifndef RUN_TIME_H
#define RUN_TIME_H
#include "hardwaredefs.h"
#include "sensor_payload.h"

class RunTimeState {
  public:
    RunTimeState():
      isAPActive(false),
      isPortalActive(false),
      startPortal(0),
      batteryPercentage(BATT_VOL_100),
      temperature(INVALID_TEMP_READING),
      humidity(INVALID_HUMIDITY_READING),
      co2(INVALID_GAS_READING),
      deviceEvents(DeviceStateEvent::DSE_None)
    {

    }

    bool isAPActive;
    bool isPortalActive;
    unsigned long startPortal;
    int batteryPercentage;
    float temperature;
    float humidity;
    uint16_t co2;
    uint deviceEvents;
     
};

RunTimeState RSTATE;

#endif
