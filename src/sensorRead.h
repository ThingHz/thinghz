#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include "deviceState.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include "utils.h"


OneWire deviceTemp(TEMP_SENSOR_PIN);
DallasTemperature tempSensor(&deviceTemp);
Adafruit_SHT31 sht31(&Wire);
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_16,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76
);

BME280I2C bme(settings);


void DSB112Init()
{
  tempSensor.begin();
}

bool bmp280Init() {
  if (!bme.begin())
  { // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find SHT31");
    //setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTDisconnected);
    return false;
  }
  //clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTDisconnected);
  return true;
}


bool shtInit()
{
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
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

  // NOTE:: is it possible that only one value fails, is it possible to trust the other value in such a case.
  // NOTE:: is nan the only sensor failure scenario
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
  RSTATE.temperature = temp + PSTATE.tempCalibration;
  RSTATE.humidity = humid;

  return true;
}

bool readBMP()
{
  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  EnvironmentCalculations::AltitudeUnit envAltUnit  =  EnvironmentCalculations::AltitudeUnit_Meters;
  EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
  float altitude = EnvironmentCalculations::Altitude(pres, envAltUnit, REF_PRESSURE, OUT_TEMP, envTempUnit);
  float seaLevel = EnvironmentCalculations::EquivalentSeaLevelPressure(BAROMETER_ALTITUDE, temp, pres, envAltUnit, envTempUnit);
  RSTATE.altitude = altitude;
  RSTATE.seaLevel = seaLevel;
  RSTATE.bmpTemp = temp;
  }


#endif
