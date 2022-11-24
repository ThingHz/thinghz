#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "utils.h"
#include <Wire.h>
#include "WiFiOTA.h"
#include <rom/rtc.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <driver/adc.h>


DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;
unsigned long startMillis = 0;


/*void sendSensorCallback() {
  DEBUG_PRINTLN("Reading new sensor data locally");
  if (readSensorData()) {
    uint8_t mac_add[6];
    WiFi.macAddress(mac_add);
  }
}*/

/*bool readSensorData()
{
  deviceState.telemetry.sensorProfile = SensorProfile::SensorNone;
  int batteryVoltage                  = readBatValue();
  deviceState.telemetry.batteryPercentage    = getBatteryPercentage(batteryVoltage);
  return true;
}*/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  DEBUG_PRINTLN("This is THingHz Range of wireless sensors");
  if (!EEPROM.begin(EEPROM_STORE_SIZE))
  {
    DEBUG_PRINTLN("Problem loading EEPROM");
  }

  bool rc = deviceState.load();
  if (!rc)
  {
    DEBUG_PRINTLN("EEPROM Values not loaded");
  }
  else
  {
    DEBUG_PRINTLN("Values Loaded");
  }
  
  DEBUG_PRINTLN("Switching to ESPNOW");
  switchToESPNowGateway(onEspNowRecv);
  captivePortal.servePortal(true);
  captivePortal.beginServer();
  analogSetAttenuation(ADC_0db);
  
}

void loop()
{

  if ((millis() - startMillis) >= (PROCESS_DATA_INTERVAL_SECS * MILLI_SECS_MULTIPLIER)) {

    processQueuedData();
    startMillis = millis();
  }
}


bool processQueuedData()
{
  
  bool connOutcome = false;
  if (!deviceState.hasUnprocessedTelemetry()) {
    DEBUG_PRINTLN("NO data to process Try Again");
    return false;
  }
  uint8_t maxModemPwrCycleRetries = MAX_MODEM_PWR_RETRIES;
  while (maxModemPwrCycleRetries > 0 ) {
    int maxWifiConnectionRetries = MAX_WIFI_CONN_RETRIES ;
    while (maxWifiConnectionRetries > 0) {
      connOutcome = switchToWifiClient(PSTATE.apSSID, PSTATE.apPass, 15); // NOTE:: why 30 secs
      if (connOutcome) {
        break;
      }
      maxWifiConnectionRetries--;
    }
    if (connOutcome) {
      DEBUG_PRINTLN("connected to wif network");
      break;
    }
    
    maxModemPwrCycleRetries--;
  }
  
  if (!hasWifiConnection()) {
    rtcState.wifiConnFailureCounter++;
    // if we have spent 5 cycles trying to connect lets just reboot the device and loose data
    if (!(rtcState.wifiConnFailureCounter % 5)) {
      storeAndReset();
    }
    return false;
  }

  if(esp_now_unregister_recv_cb() != ESP_OK){
      DEBUG_PRINTLN("cant unregister ESP nOW cb");
  }

  bool rc = cloudTalk.sendSensorData(deviceState);
  if (!rc) {
    DEBUG_PRINTLN("send data failed");
  }

  storeAndReset();

  return true;
}

float readBatValue()
{
  //formula for VD1 = 1M/(3.9M+1M)
  int adcVal = analogRead(BATTERY_VOL_PIN);
  float batVol = adcVal * 0.00127; //finalVolt = (1/1024)(1/VD)    external VD [VD1 = 3.3Mohm/(1Mohm+3.3Mohm)]
  DEBUG_PRINTF("adcVal %d\n", adcVal);
  DEBUG_PRINTF("batteryVoltage %.1f\n", batVol);
  return batVol;
}

void storeAndReset()
{
  // store persistent state
  bool storeValue = deviceState.store();
  if (!storeValue) {
    DEBUG_PRINTLN("Error storing values to EEPROM");
  } else {
    DEBUG_PRINTLN("Values Stored");
  }
  SPIFFS.end();
  esp_restart();
}

void onEspNowRecv(const uint8_t *mac_add, const uint8_t *data, int data_len)
{
  DEBUG_PRINTLN("Received message from espnow node");

  if (data_len < sizeof(SensorPayloadTHCO2)) {
    DEBUG_PRINTLN("Bad data from espnow");
    return;
  }

  DEBUG_PRINTF("size of incoming data is : %d, and size of struct is %d \n",  data_len, sizeof(SensorPayloadTHCO2));

  SensorPayloadTHCO2 *payload = (SensorPayloadTHCO2*) data;

  DEBUG_PRINTF("battery Percentage %d",payload->batteryPercentage);
  DEBUG_PRINTF("Temperatue%.1f",payload->temperature);
  DEBUG_PRINTF("SensorProfile%d",payload->sensorProfile);
  
  
  deviceState.enqueSensorPayload(convertToStringWithoutColons(mac_add).c_str(), payload);
}
