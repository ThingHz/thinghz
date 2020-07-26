#ifndef UTILS_H
#define UTILS_H

#include "deviceState.h"
/**
 * @brief:
 * Helper function to get the RSSI percentage from the RSSI in db of available Networks
 * @param:
 * RSSI in db 
 * @return:
 * rssi in percentage   
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


String DSEEnumToError(int enumValue)
{
  String criticalDevMessage = "";
  switch (enumValue) {
    case DeviceStateEvent::DSE_SHTDisconnected:
      criticalDevMessage = "TH Sensor is not connected";
      break;
    case DeviceStateEvent::DSE_SHTFaulty:
      criticalDevMessage = "TH Sensor is faulty";
      break;
    case DeviceStateEvent::DSE_TMPDisconnected:
      criticalDevMessage = "Temperature Sensor is not connected";
      break;
    case DeviceStateEvent::DSE_TMPFaulty:
      criticalDevMessage = "Temperature Sensor is Faulty";
      break;
    case DeviceStateEvent::DSE_DSBFaulty:
      criticalDevMessage = "DSB112 Temperature Sensor is Faulty";    
    default :
      criticalDevMessage = "";
      break;
  }
  return criticalDevMessage;
}

String cirticialMsg()
{
  if (RSTATE.deviceEvents == 0) {
    return String("All Healthy");
  }

  String criticalDevMessage = "";

  int flagMask = DSE_SHTDisconnected; // first error flag
  while (RSTATE.deviceEvents & flagMask) {
    criticalDevMessage += DSEEnumToError(RSTATE.deviceEvents & flagMask);
    criticalDevMessage += "\n";
    flagMask = flagMask << 1;
  }

  return criticalDevMessage;
}

bool checkSetBit(int state, int setBit){
  bool isBitSet = state & (1<<setBit);
  if(!isBitSet){
      return false;
    }
    return true;
}

float reportedRoomTemp()
{
  if (checkSetBit(RSTATE.deviceEvents,0) || checkSetBit(RSTATE.deviceEvents,1)) {
  return RSTATE.roomTemp;
  }
  return RSTATE.roomTempTH;
}

#endif