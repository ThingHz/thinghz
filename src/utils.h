#ifndef UTILS_H
#define UTILS_H

#include "hardwareDefs.h"
#include <SPIFFS.h>

// return battery voltage in "V" units


/**
   @brief:
   Helper function to get the Battery percentage.
   Battery voltage is mapped to 0 to 100 factor
   @param:
   Battery Voltage in float
   @return:
   battery percentage in int
*/
int getBatteryPercentage(const float battVoltage) {
  int maxVal = BATT_VOL_100 * 100; // scale by 100
  int minVal = BATT_VOL_0 * 100; // scale by 100
  int percentage = 0;
  if (battVoltage > maxVal) {
    maxVal = battVoltage;
  }
  percentage = map(int(battVoltage * 100), minVal, maxVal, 0, 100);
  if (percentage < 0) {
    percentage = 0;
  }  if (percentage > 100) {
    percentage = 100;
  }
  return percentage;
}


/**
   @brief:
   Helper function to get the RSSI percentage from the RSSI in db of available Networks
   @param:
   RSSI in db
   @return:
   rssi in percentage
*/

int getRSSIasQuality(int RSSI) {
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

/**
   @brief:
   Helper function to get if there are multiple faliure.
   @todo:
   Do we need this we have another helper funtion for the same purpose
   @param:
   Device State enum
   @return:
   true for multiple device event
*/

bool isMultiDeviceEvent(int n) {
  if (n == 0) {
    return false;
  }
  while (n != 1)
  {
    n = n / 2;
    if (n % 2 != 0 && n != 1) {
      return false;
    }
  }
  return true;
}

/**
   @brief:
   Checks for the deviceStateEvent set bits.
   @param:
   deviceEvent      bit position
   @return:
   true if bit is set
*/

bool checkSetBit(int state, int setBit) {
  return (state & (1 << setBit));
}


/**
   @brief:
   Checks for the deviceStateEvent set bits.
   @param:
   deviceEvent      bit position
   @return:
   true if bit is set
*/

bool testBit(uint &bits, int bit)
{
  return (bits & bit);
}

/**
   @brief:
   sets the bit.
   @param:
   deviceEvent      bit position
*/
void setBit(uint &bits, int bit)
{
  bits |= bit;
}

/**
   @brief:
   clears the bit.
   @param:
   deviceEvent      bit position
*/

void clearBit(uint &bits, int bit)
{
  bits &= (~bit);
}


/**
   @brief:
   Prepares a remote file name for ota
   @return:
   String
*/

String prepareRemoteFWFileName(uint8_t devType, uint8_t hwRev, uint8_t newFwRev)
{
  char retString[20] = { 0 };
  snprintf(retString, sizeof(retString), "%d-%d-%d.bin", devType, hwRev, newFwRev);
  return String(retString);
}


/**
   @brief:
   Checks if file already exists in SPIFFS
   @param:
   String filename
   @return:
   true if exists either false
*/

bool isFileExistInSpiff(String fileName) {
  if (SPIFFS.exists(fileName.c_str())) {
    return true;
  }
  return false;
}

/**
   @brief:
   Converts IP address to string
   @param:
   referance to IP address class
   @return:
   String IP address
*/
String IpAddressToString(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3])  ;
}

/**
   @brief:
   reset modem
*/
void modemReset() {
  DEBUG_PRINTLN("Reseting modem");
  digitalWrite(MODEM_RST, HIGH);
}


/**
   @brief:
   restart modem
*/
void modemRestart() {
  DEBUG_PRINTLN("Restart modem");
  digitalWrite(MODEM_POWER_ON, HIGH);
  delay(100);
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(100);
  digitalWrite(MODEM_PWKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWKEY, HIGH);
}

#endif
