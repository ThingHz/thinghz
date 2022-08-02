#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "hardwaredefs.h"
#include "Adafruit_CCS811.h"
#include "sensor_payload.h"
#include "runTime.h"

Adafruit_CCS811 ccs;


/**
   @brief:
   Class for runtime Device status
*/

OneWire deviceTemp(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&deviceTemp);
Adafruit_SHT31 sht31(&Wire);


void DSB112Init()
{
  tempSensor.begin();
}

bool ccsInit(){
    if (!ccs.begin())
    {
        DEBUG_PRINTLN("Couldn't find CCS");
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_CCSFaulty);
        return false;
    }
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_CCSFaulty);
    return true;
}

bool isCCSAvailable()
{
  return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_CCSFaulty);
}

bool readCCS(){
    unsigned long startTime;
    startTime = millis();
    while(!ccs.available()&&millis()-startTime < CCS_WARM_DURATION);
    if(!ccs.available()){
        return false;
        DEBUG_PRINTLN("CCS not available");
    } 
     if(!ccs.readData()){
        uint16_t co2Value = ccs.geteCO2();
        DEBUG_PRINTF("Co2 value %u\n", (unsigned)co2Value);
        RSTATE.co2 = co2Value; 
        }
    return true;
    }
    


bool shtInit()
{
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find SHT31");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
  return true;
}

bool isSHTAvailable()
{
  return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
}

bool isSHTWorking()
{
  auto available = isSHTAvailable();
  auto notFaulty = !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
  return (available && notFaulty);
}

bool readDSB112()
{
  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);
  DEBUG_PRINTF("DSB temprature %.1f", temp);
  if (isnan(temp) || (int)temp < -50) {
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_DSBFaulty);
    DEBUG_PRINTLN("failed to read DSB temperature");
    return false;
  }
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_DSBFaulty);
  RSTATE.temperature = temp;
  return true;
}

bool readSHT()
{
  auto tempHumid = sht31.readTempHumidity();

  float temp = tempHumid.first;

  float humid = tempHumid.second;

  if (isnan(temp))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read temperature");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  if (isnan(humid))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read humidity");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  RSTATE.temperature = temp;
  RSTATE.humidity = humid;

  return true;
}


#endif
