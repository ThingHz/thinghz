#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT31.h"
#include "deviceState.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "utils.h"
#include "SparkFun_SCD4x_Arduino_Library.h" 


Adafruit_SHT31 sht31(&Wire);

/**
   @brief:
   Initialise SHT31
   @return:
   true when initialisation success else set the deviceStateevent and return false
*/
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


/**
   @brief:
   Initialise SCD40
   @return:
   true when initialisation success else set the deviceStateevent and return false
*/
bool scdInit(SCD4x *scd_sensor)
{
  if (!scd_sensor->begin())
  {
    DEBUG_PRINTLN("Couldn't find SCD");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_GASFaulty);
    return false;
  }
  scd_sensor->setSensorAltitude(ALTITUDE_FOR_SCD); 
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_GASFaulty);
  return true;
}


/**
   @brief:
   test the bit status 
   @return:
   true when initialisation success and test the deviceStateEvent
*/
bool isSHTAvailable()
{
  return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
}


/**
   @brief:
   test the bit status 
   @return:
   true when initialisation success and test the deviceStateEvent
*/
bool isSCDAvailable(){
    return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_GASFaulty);  
}


bool isSHTWorking()
{
  auto available = isSHTAvailable();
  auto notFaulty = !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
  return (available && notFaulty);
}


/**
   @brief:
   read SHT value 
   @return:
   true when redings are success
*/
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
  RSTATE.temperature = temp + PSTATE.tempCalibration;
  RSTATE.humidity = humid + PSTATE.humidCalibration;

  return true;
}


/**
   @brief:
   read SCD value 
   @return:
   true when readings are success
*/
bool readSCD(SCD4x *scd_sensor){
  if (scd_sensor->readMeasurement())
  { 
    uint16_t carbonValue = scd_sensor->getCO2();
    RSTATE.carbon = carbonValue + PSTATE.carbonCalibration;
    return true;
  }
  DEBUG_PRINTLN("Failed to get CO2 Values");
  return false;
}


#endif
