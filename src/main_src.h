#ifndef MAINSRC_H
#define MAINSRC_H

#include "sensorRead.h"
#include "cloudInteractGSM.h"
#include "deviceState.h"
#include <WiFi.h>

void readSensors();
bool isNetworkConnected(TinyGsm*);
void setHardwarePinsAndDirection();
void setCerts(SSLClient* secureClient);
sendPayloadUsingWifi();
sensdPayloadUsingGSM();



#endif