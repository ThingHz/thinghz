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
   Power On modem
*/
void modemPowerKeyToggle() {
  DEBUG_PRINTLN("Power On modem");
  digitalWrite(MODEM_PWKEY, LOW);
  delay(100);
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWKEY, LOW);
}

/**
   @brief:
   toggle light state
*/
void toggleLightState(uint8_t lightState_1, uint8_t lightState_2, uint8_t lightState_3, uint8_t lightState_4) {
  RSTATE.light_state_1 = !lightState_1;
  RSTATE.light_state_2 = !lightState_2;
  RSTATE.light_state_3 = !lightState_3;
  RSTATE.light_state_4 = !lightState_4;
  PSTATE.light_state_1 = !lightState_1;
  PSTATE.light_state_2 = !lightState_2;
  PSTATE.light_state_3 = !lightState_3;
  PSTATE.light_state_4 = !lightState_4;
  //RSTATE.light_state = !RSTATE.light_state;
  DEBUG_PRINTF("toggle light1 state to: %d and light2 state to: %d\n", !lightState_1, !lightState_2);
  digitalWrite(RELAY_PIN_1, RSTATE.light_state_1);
  digitalWrite(RELAY_PIN_2, RSTATE.light_state_2);
  digitalWrite(RELAY_PIN_3, RSTATE.light_state_3);
  digitalWrite(RELAY_PIN_4, RSTATE.light_state_4);
}

void currentLightState() {
  RSTATE.light_state_1 = digitalRead(RELAY_PIN_1);
  RSTATE.light_state_2 = digitalRead(RELAY_PIN_2);
  RSTATE.light_state_3 = digitalRead(RELAY_PIN_3);
  RSTATE.light_state_4 = digitalRead(RELAY_PIN_4);
  DEBUG_PRINTF("Light 1 current state is: %d, for Light 2 its: %d, for Light 3 its: %d, for Light 4 its: %d\n",
               RSTATE.light_state_1,
               RSTATE.light_state_2,
               RSTATE.light_state_3,
               RSTATE.light_state_4);
}


/**
   @brief:
   blink Signal Led
*/
void blinkSignalLed(uint8_t ledState)
{
    digitalWrite(SIG_PIN, ledState);
}


#endif


