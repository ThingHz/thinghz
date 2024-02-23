#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT4x.h"
#include "deviceState.h"
#include "utils.h"
#include "SparkFun_SCD4x_Arduino_Library.h" 
#include "BH1750.h"


Adafruit_SHT4x sht4 = Adafruit_SHT4x();
BH1750 lightMeter(0x23);

/**
   @brief:
   Initialise SHT31
   @return:
   true when initialisation success else set the deviceStateevent and return false
*/
bool shtInit()
{
  if (!sht4.begin(&Wire))
  { // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find SHT4x");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
  return true;
}


/**
   @brief:
   Initialise BH1750
   @return:
   true when initialisation success else set the deviceStateevent and return false
*/
bool lightInit()
{
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
  { // Set to 0x45 for alternate i2c addr
    DEBUG_PRINTLN("Couldn't find BH1750");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_LIGHTFaulty);
    return false;
  }
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_LIGHTFaulty);
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

/**
   @brief:
   test the bit status 
   @return:
   true when initialisation success and test the deviceStateEvent
*/
bool isLightAvailable(){
    return !testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_LIGHTFaulty);  
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
bool readSHT() {
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  float temp_sht = temp.temperature;

  float humid_sht = humidity.relative_humidity;

  if (isnan(temp_sht))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read temperature");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  if (isnan(humid_sht))
  { // check if 'is not a number'
    DEBUG_PRINTLN("Failed to read humidity");
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SHTFaulty);
    return false;
  }
  RSTATE.temperature = temp_sht; 
  RSTATE.humidity = humid_sht;

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

/**
   @brief:
   read BH1750 value 
   @return:
   true when readings are success
*/
bool readLight(){
  if (lightMeter.measurementReady())
  { 
    float luxValues = lightMeter.readLightLevel();
    RSTATE.lux = luxValues;
    return true;
  }
  DEBUG_PRINTLN("Failed to get lux Values");
  return false;
}
#endif
