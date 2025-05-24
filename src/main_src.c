#include "main_src.h"

void readSensors(){
  if (!isSHTAvailable())
    {
      DEBUG_PRINTLN(F("SHT Not connected, initialising again"));
      shtInit();
    }
    else
    {
      readSHT();
      DEBUG_PRINTF("Temperature Value: %1f, Humidity Value: %1f\n", RSTATE.temperature, RSTATE.humidity);
    }
    if (!isLightAvailable)
    {
      DEBUG_PRINTLN(F("BH1750 Not connected, initialising again"));
      lightInit();
    }
    else
    {
      readLight();
      DEBUG_PRINTF("Lux Value: %.1f\n", RSTATE.lux);
    }
}

void isNetworkConnected(TineGSM* modem){
  return (modem.isGprsConnected() ||  WiFi.isConnected());
}

void setHardwarePinsAndDirection(){
  pinMode(SIG_PIN, OUTPUT);
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  digitalWrite(RELAY_PIN_1, PSTATE.light_state_1);
  digitalWrite(RELAY_PIN_2, PSTATE.light_state_2);
  digitalWrite(RELAY_PIN_3, PSTATE.light_state_3);
  digitalWrite(RELAY_PIN_4, PSTATE.light_state_4);
  modemPowerKeyToggle();
}

void setCerts(){
  secureClient.setCACert(cacert);
  secureClient.setCertificate(clientcert);
  secureClient.setPrivateKey(clientkey);
  secureClient.setTimeout(3000);
}

