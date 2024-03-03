#ifndef MODEM_FUNC_H
#define MODEM_FUNC_H

#include "hardwareDefs.h"
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
  digitalWrite(MODEM_FLIGHT, HIGH);
}

/**
        @brief: restart Modem
        @param: TinyGsm pointer
        @return: true when everything goes write
*/

bool restartModem() {
  int ret = modem.restart();
  String modemInfo = modem.getModemInfo();
  DEBUG_PRINTF("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem.getSimStatus());
  modem.setNetworkMode(2);
  if (modem.getSimStatus() == 0) {
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
  } else {
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
  }

  return ret;
}

/**
      @brief: Initialise Modem
      @param: TinyGsm pointer
      @return: true when everything goes write
*/

bool initialiseModem() {
  DEBUG_PRINTLN("initialising Modem");
  int ret = modem.init();
  String modemInfo = modem.getModemInfo();
  DEBUG_PRINTF("modemInfo: %s\n SimStatus: %d\n", modemInfo.c_str(), modem.getSimStatus());
  modem.setNetworkMode(2);
  if (modem.getSimStatus() == 0) {
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
  } else {
  }

  return ret;
}

/**
      @brief: Chgeck Network or Restart modem
      @param: TinyGsm pointer
      @return: true when everything goes right
*/

bool checkNetwork() {
  static int gsm_retries = 0;
  if (!modem.isNetworkConnected()) {
    DEBUG_PRINTLN("Network not available");
    if (!modem.waitForNetwork()) {
      delay(4000);
    }
    gsm_retries++;
    if (gsm_retries >= RSTATE.gsmConnectionRetries) {
      restartModem(&modem);
    }
    setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_NoNetwork);
    return false
  }
  clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_NoNetwork);

  return true;
}

/**
      @brief: Connect Gprs
      @param: TinyGsm pointer
      @return: true when everything goes right
*/
bool connectGPRS() {
  if (modem.isNetworkConnected() && !modem.isGprsConnected()) {
    DEBUG_PRINTLN("GPRS not connected");
    modem.gprsConnect("airtelgprs.com");
  }
  if (modem.isGprsConnected()) {
    String dateTime = modem.getGSMDateTime(DATE_FULL);
    DEBUG_PRINTF("Current Time : %s", dateTime);
    return true;
  }
  return false;
}
#endif