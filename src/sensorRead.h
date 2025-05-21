#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include "Adafruit_SHT4x.h"
//#include "Adafruit_SHT31.h"
#include "deviceState.h"
#include "utils.h"
#include "BH1750.h"

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
BH1750 lightMeter(0x23);

bool shtInit();
bool lightInit();
bool isSHTAvailable();
bool isLightAvailable();
bool isSHTWorking();
bool readSHT();
bool readLight();
#endif
