#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "sensorRead.h"
#include "utils.h"
#include <Wire.h>
#include "WiFiOTA.h"
#include "espBLE.h"
#include <rom/rtc.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <driver/adc.h>
#include <Protocentral_FDC1004.h>

DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;
FDC1004 FDC;
ESPble espBle;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
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

  DEBUG_PRINTF("The reset reason is %d\n", (int)rtc_get_reset_reason(0));
  if ((int)rtc_get_reset_reason(0) == 1)
  { // =  SW_CPU_RESET
    RSTATE.isPortalActive = true;
    if (!APConnection(AP_MODE_SSID))
    {
      DEBUG_PRINTLN("Error Setting Up AP Connection");
      return;
    }
    delay(100);
    captivePortal.servePortal(true);
    captivePortal.beginServer();
    delay(100);
  }

  if (!RSTATE.isPortalActive)
  {
    if (setCpuFrequencyMhz(80))
    {
      DEBUG_PRINTLN("Set to 80MHz");
    }
  }

  pinMode(SIG_PIN, OUTPUT);
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(CONFIG_PIN, INPUT);
  pinMode(VOLTAGE_DIV_PIN, OUTPUT);
  digitalWrite(VOLTAGE_DIV_PIN, LOW);
  analogSetAttenuation(ADC_0db);
  shtInit();
  DSB112Init();
  if (!isSHTAvailable())
  {
    DEBUG_PRINTLN("SHT Not connected");
  }

  if (readSHT())
  {
    DEBUG_PRINTF("Temperature:%.1f\n", RSTATE.temperature);
    DEBUG_PRINTF("Humidity:%.1f\n", RSTATE.humidity);
    if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
    {
      goToDeepSleep();
    }
  }

  if (readDSB112())
  {
    DEBUG_PRINTF("Temperature:%.1f\n", RSTATE.temperature);
    if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
    {
      goToDeepSleep();
    }
  }

  if (espBle.setupBLE(RSTATE.isPortalActive, PSTATE.isBLE))
  {
    blinkLed();
    goToDeepSleep();
  }

  if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
  {
    rtcState.missedDataPoint++;
    goToDeepSleep();
    DEBUG_PRINTLN("Error connecting to WiFi");
  }

  digitalWrite(VOLTAGE_DIV_PIN, LOW);

  RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
}

void loop()
{
  if (!RSTATE.isPortalActive)
  {
    DEBUG_PRINTF("Mode Activated:%d", PSTATE.isBLE);  
    if (espBle.setupBLE(RSTATE.isPortalActive, PSTATE.isBLE))
    {
      DEBUG_PRINTLN("BLE mode selscted");
      blinkLed();
      goToDeepSleep();
    }
    if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300) && !PSTATE.isBLE)
    {
      DEBUG_PRINTLN("Error connecting to WiFi");
      goToDeepSleep();
    }

    if (cloudTalk.sendPayload())
      blinkLed();

    DEBUG_PRINTLN("Going to sleep");
    goToDeepSleep();
  }

  if (millis() - RSTATE.startPortal >= SECS_PORTAL_WAIT * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
  {
    RSTATE.isPortalActive = false;
  }
}

void blinkLed()
{
  digitalWrite(SIG_PIN, HIGH);
  delay(500);
  digitalWrite(SIG_PIN, LOW);
}

bool checkAlarm(uint8_t sProfile)
{
  bool isAlarm = false;
  DEBUG_PRINTLN(sProfile);
  switch (sProfile)
  {
  case SensorProfile::SensorNone:
    isAlarm = true;
    break;
  case SensorProfile::SensorTemp:
  {
    if (((int)RSTATE.temperature < PSTATE.targetTempMax &&
         (int)RSTATE.temperature > PSTATE.targetTempMin) &&
        rtcState.wakeUpCount < MAX_WAKEUP_COUNT)
    {
      DEBUG_PRINTLN("Value is in range going to sleep");
      DEBUG_PRINTF("Wake Up Count %d\n", rtcState.wakeUpCount);
      rtcState.wakeUpCount++;
      rtcState.isEscalation = 0;
      isAlarm = false;
    }
    else if (rtcState.wakeUpCount >= MAX_WAKEUP_COUNT)
    {
      DEBUG_PRINTLN("has Reached max offline cout now log data");
      rtcState.wakeUpCount = 0;
      isAlarm = true;
    }
    else
    {
      DEBUG_PRINTLN("Values not in range going to sleep");
      rtcState.isEscalation++;
      rtcState.wakeUpCount = 0;
      isAlarm = true;
    }
  }
  break;
  case SensorProfile::SensorTH:
  {
    DEBUG_PRINTLN(rtcState.wakeUpCount);
    DEBUG_PRINTLN(PSTATE.targetTempMax);
    DEBUG_PRINTLN(PSTATE.targetTempMin);
    if ((((int)RSTATE.temperature < PSTATE.targetTempMax &&
          (int)RSTATE.temperature > PSTATE.targetTempMin)) &&
        (rtcState.wakeUpCount < MAX_WAKEUP_COUNT))
    {
      DEBUG_PRINTLN("Value is in range going to sleep");
      DEBUG_PRINTF("Wake Up Count %d\n", rtcState.wakeUpCount);
      rtcState.wakeUpCount++;
      rtcState.isEscalation = 0;
      isAlarm = false;
    }
    else if (rtcState.wakeUpCount >= MAX_WAKEUP_COUNT)
    {
      DEBUG_PRINTLN("has Reached max offline cout now log data");
      rtcState.wakeUpCount = 0;
      isAlarm = true;
    }
    else
    {
      DEBUG_PRINTLN("Values not in range Log data");
      isAlarm = true;
      rtcState.wakeUpCount = 0;
      rtcState.isEscalation++;
    }
  }
  break;
  default:
    if (((int)RSTATE.temperature < PSTATE.targetTempMax &&
         (int)RSTATE.temperature > PSTATE.targetTempMin) &&
        rtcState.wakeUpCount < MAX_WAKEUP_COUNT)
    {
      DEBUG_PRINTLN("Value is in range going to sleep");
      DEBUG_PRINTF("Wake Up Count %d\n", rtcState.wakeUpCount);
      rtcState.wakeUpCount++;
      rtcState.isEscalation = 0;
      isAlarm = false;
    }
    else if (rtcState.wakeUpCount >= MAX_WAKEUP_COUNT)
    {
      DEBUG_PRINTLN("has Reached max offline cout now log data");
      rtcState.wakeUpCount = 0;
      isAlarm = true;
    }
    else
    {
      DEBUG_PRINTLN("Values not in range going to sleep");
      isAlarm = true;
      rtcState.wakeUpCount = 0;
      rtcState.isEscalation++;
    }
    break;
  }

  return isAlarm;
}

void goToDeepSleep()
{

  bool rc = deviceState.store();
  if (!rc)
  {
    DEBUG_PRINTLN("EEPROM Values not loaded");
  }
  else
  {
    DEBUG_PRINTLN("Values Loaded");
  }
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(VOLTAGE_DIV_PIN, HIGH);
  DEBUG_PRINTLN("going to sleep");
  //btStop();
  //esp_wifi_stop();
  //esp_bt_controller_disable();
  //adc_power_off();
  esp_sleep_enable_timer_wakeup(SECS_MULTIPLIER_DEEPSLEEP * MICRO_SECS_MULITPLIER);
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,0);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
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
