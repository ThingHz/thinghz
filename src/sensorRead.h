#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include "deviceState.h"
#include <OneWire.h>
#include "SensorPayload.h"
#include <DallasTemperature.h>

OneWire           deviceTemp(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&deviceTemp);
Adafruit_SHT31    sht31(&Wire);

bool testBit(uint &bits, int bit)
{
  return (bits & bit);
}

void setBit(uint &bits, int bit)
{
  bits |= bit;
}

void clearBit(uint &bits, int bit)
{
  bits &= (~bit);
}

void DSB112Init() {
  tempSensor.begin();
}

bool shtInit()
{
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find SHT31");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTDisconnected);
    return false;
  }
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTDisconnected);
  return true;
}

bool isSHTAvailable()
{
  return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTDisconnected);
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
  DEBUG_PRINTF("DSB temprature %.1f",temp);
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

  // NOTE:: is it possible that only one value fails, is it possible to trust the other value in such a case.
  // NOTE:: is nan the only sensor failure scenario
  if (isnan(temp)) {  // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read temperature");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  if (isnan(humid)) {  // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read humidity");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  RSTATE.temperature = temp + PSTATE.tempCalibration;
  RSTATE.humidity = humid;

  return true;
}


#endif
