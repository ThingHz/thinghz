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
  pinMode(SIG_PIN,              OUTPUT);
  pinMode(TEMP_SENSOR_PIN,      INPUT);
  pinMode(CONFIG_PIN,           INPUT);
  pinMode(VOLTAGE_DIV_PIN,      OUTPUT);
  digitalWrite(VOLTAGE_DIV_PIN,LOW);
  shtInit();
  DSB112Init();
  
  if (readDSB112()) {
      if (RSTATE.temperature > rtcState.targetTempMax|| RSTATE.temperature < rtcState.targetTempMin) {
          DEBUG_PRINTLN("Escalation occured - log the data");
          return;
      }else if (rtcState.wakeUpCount>=MAX_WAKEUP_COUNT) {
          DEBUG_PRINTLN("has reacehed max offline count- now log the data");
          rtcState.wakeUpCount = 0;
          return;
      }else {
        rtcState.wakeUpCount++;
        goToDeepSleep();
      }
  }
  
  if (!isSHTAvailable()) {
    DEBUG_PRINTLN("SHT Not connected");
  }

  attachInterrupt(digitalPinToInterrupt(CONFIG_PIN), ISR_Config_Button,   FALLING);  //pin Change High to Low
  RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
  if (RSTATE.isPortalActive == true) {
      captivePortal.servePortal(true);
      captivePortal.beginServer();
      if(!APConnection(AP_MODE_SSID)){
        DEBUG_PRINTLN("Error Setting Up AP Connection");
        return;
      }
      RSTATE.isPortalActive = false;
  }
  if(!reconnectWiFi((PSTATE.apSSID).c_str(),(PSTATE.apPass).c_str(),300)){
      goToDeepSleep();
      DEBUG_PRINTLN("Error connecting to WiFi");
  }
}


void loop() {
  if (RSTATE.isPortalActive == true) {
       if(!APConnection(AP_MODE_SSID)){
        DEBUG_PRINTLN("Error Setting Up AP Connection");
        return;
      } 
      captivePortal.servePortal(true);
      captivePortal.beginServer();
      RSTATE.isPortalActive = false;
  }
  if (!(WiFi.softAPgetStationNum() > 0)) {
      if (!reconnectWiFi((PSTATE.apSSID).c_str(),(PSTATE.apPass).c_str(),300)) {
          DEBUG_PRINTLN("Error connecting to WiFi");
    }
      if (!cloudTalk.sendPayload()) {
          DEBUG_PRINTLN("Error Sending Payload");
      }
      digitalWrite(SIG_PIN,HIGH);
      delay(500);
      digitalWrite(SIG_PIN,LOW);
      DEBUG_PRINTLN("Going to sleep");
      goToDeepSleep();
     
  }
}

void goToDeepSleep(){
    esp_sleep_enable_timer_wakeup(SECS_MULTIPLIER_DEEPSLEEP*MICRO_SECS_MULITPLIER);
      //esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,0);   
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();
}
  
