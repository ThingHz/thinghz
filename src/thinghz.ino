#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "sensorRead.h"
#include "utils.h"
#include "WiFiOTA.h"

DeviceState         state;
DeviceState& deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;

void IRAM_ATTR ISR_Config_Button() {
  static unsigned long buttonDebounce = 0;
  if (millis() - buttonDebounce > MINIMUM_DEBOUNCE_TIME) {
    RSTATE.isPortalActive = true;
  }
  buttonDebounce = millis();
}


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
DEBUG_PRINTLN("Started ThingHz Wireless Temperature Sensor");
if (!EEPROM.begin(EEPROM_STORE_SIZE)) {
    DEBUG_PRINTLN("Problem loading EEPROM");
  }

if (!SPIFFS.begin(true)) {
    DEBUG_PRINTLN("An Error has occurred while mounting SPIFFS, fw upgrades will not work");
  } else {
    DEBUG_PRINTLN("sucessfully mouted spiffs");
  }
  pinMode(SIG_PIN,              OUTPUT);
  pinMode(TEMP_SENSOR_PIN,      INPUT);
  pinMode(CONFIG_PIN,           INPUT);
  /*if (PSTATE.isOtaAvailable == 1) {
    PSTATE.isOtaAvailable = 0;
    deviceState.store();
    captivePortal.endPortal();
    bool rc = performOTA();
    if (!rc) {
      PSTATE.isOtaAvailable = 1;
      deviceState.store();
    }
  }*/
  shtInit();
  DSB112Init();
  if (!isSHTAvailable()) {
    DEBUG_PRINTLN("SHT Not connected");
  }
  attachInterrupt(digitalPinToInterrupt(CONFIG_PIN), ISR_Config_Button,   FALLING);  //pin Change High to Low
  RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
  if (RSTATE.isPortalActive == true) {
      captivePortal.servePortal(true);
      captivePortal.beginServer();
      if(!APConnection(WAN_WIFI_SSID_DEFAULT)){
        DEBUG_PRINTLN("Error Setting Up AP Connection");
        return;
      }
      RSTATE.isPortalActive = false;
  }
  if(!reconnectWiFi((PSTATE.apSSID).c_str(),(PSTATE.apPass).c_str(),300)){
      DEBUG_PRINTLN("Error connecting to WiFi");
  }
}


void loop() {
  if (RSTATE.isPortalActive == true) {
       if(!APConnection(WAN_WIFI_SSID_DEFAULT)){
        DEBUG_PRINTLN("Error Setting Up AP Connection");
        return;
      } 
      captivePortal.servePortal(true);
      captivePortal.beginServer();
      RSTATE.isPortalActive = false;
  }
  if (!(WiFi.softAPgetStationNum() > 0)&&millis()>300*MILLI_SECS_MULTIPLIER) {
      if (!reconnectWiFi((PSTATE.apSSID).c_str(),(PSTATE.apPass).c_str(),300)) {
          DEBUG_PRINTLN("Error connecting to WiFi");
    }
      if (!cloudTalk.sendPayload()) {
          DEBUG_PRINTLN("Error Sending Payload");
      }

      esp_sleep_enable_timer_wakeup(SECS_MULTIPLIER_DEEPSLEEP*MICRO_SECS_MULITPLIER);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,0);   
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
      esp_deep_sleep_start();
  }
}
  

