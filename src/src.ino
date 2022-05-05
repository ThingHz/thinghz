#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "sensorRead.h"
#include "utils.h"
#include <Wire.h>
#include "WiFiOTA.h"
#include <rom/rtc.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include "oledState.h"

DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
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
  #ifdef OLED_DISPLAY
    initDisplay();
    clearDisplay();
  #endif
  DEBUG_PRINTF("The reset reason is %d\n", (int)rtc_get_reset_reason(0));
  if ( ((int)rtc_get_reset_reason(0) == 12) || ((int)rtc_get_reset_reason(0) == 1))  { // =  SW_CPU_RESET
    RSTATE.isPortalActive  = true;
    if (!APConnection(AP_MODE_SSID)) {
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
  bmp280Init();

  switch (DEVICE_SENSOR_TYPE)
  {
    case SensorTH:
      {
        if (!isSHTAvailable())
          {
            DEBUG_PRINTLN("SHT Not connected");
          }

        if (readSHT())
          {
            DEBUG_PRINTF("Temperature:%.1f\n", RSTATE.temperature);
            DEBUG_PRINTF("Humidity:%.1f\n", RSTATE.humidity);
            RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
            #ifdef OLED_DISPLAY
              RSTATE.displayEvents = DisplayTempHumid;
              drawDisplay(RSTATE.displayEvents);
            #endif
            #ifndef ESP_NOW
              if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
                  {
                    goToDeepSleep();
                  }
            #endif
          }
      }
      break;
    case SensorTemp:
      {
        if (readDSB112())
          {
            DEBUG_PRINTF("Temperature:%.1f\n", RSTATE.temperature);
            RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
            #ifdef OLED_DISPLAY
              RSTATE.displayEvents = DisplayTemp;
              drawDisplay(RSTATE.displayEvents);
            #endif
            #ifndef ESP_NOW
              if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
                  {
                    goToDeepSleep();
                  }
            #endif
          }        
      }
      break;
    case SensorBMP:
      {
        if(readBMP())
          {
            DEBUG_PRINTF("TemperatureBMP:%.1f\n", RSTATE.bmpTemp);
            DEBUG_PRINTF("Altitude:%.1f\n", RSTATE.altitude);
            DEBUG_PRINTF("SeaLevel:%.1f\n", RSTATE.seaLevel);
            RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
            #ifdef OLED_DISPLAY
              clearDisplay();
              RSTATE.displayEvents = DisplayTempBMP;
              drawDisplay(RSTATE.displayEvents);
            #endif
            
            #ifndef ESP_NOW
            if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
              {
                goToDeepSleep();
              }
            #endif
          }
      }
      break;
    case SensorBMPTH:
      {
        if(readBMP() && readSHT())
          {
            DEBUG_PRINTF("TemperatureBMP:%.1f\n", RSTATE.bmpTemp);
            DEBUG_PRINTF("Altitude:%.1f\n", RSTATE.altitude);
            DEBUG_PRINTF("SeaLevel:%.1f\n", RSTATE.seaLevel);
            DEBUG_PRINTF("humidity:%.1f\n",RSTATE.humidity);
            RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
            #ifdef OLED_DISPLAY
              clearDisplay();
              RSTATE.displayEvents = DisplayTempHumidBMP;
              drawDisplay(RSTATE.displayEvents);
            #endif
            #ifndef ESP_NOW
            if (!RSTATE.isPortalActive && !checkAlarm(DEVICE_SENSOR_TYPE))
              {
                goToDeepSleep();
              }
            #endif
          }
      }
      break;
      case Gateway:
        switchToESPNowGateway(onEspNowRecv);
      default:
        #ifdef OLED_DISPLAY
          clearDisplay();
          RSTATE.displayEvents = DisplayNone;
          drawDisplay(RSTATE.displayEvents);
        #endif      
      break;
  }
    
  #ifdef ESP_NOW
    if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
        {
          rtcState.missedDataPoint++;
          goToDeepSleep();
          DEBUG_PRINTLN("Error connecting to WiFi");
        }
  #endif
  

  digitalWrite(VOLTAGE_DIV_PIN, LOW);

}

void loop()
{
  if (!RSTATE.isPortalActive)
  {
    if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
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


void onEspNowRecv(u8 *mac_add, u8 *data, u8 data_len)
{
  DEBUG_PRINTLN("Received message from espnow node");

  if (data_len < sizeof(SensorPayloadTemp)) {
    DEBUG_PRINTLN("Bad data from espnow");
    return;
  }

  DEBUG_PRINTF("mac Address %s", convertToStringWithoutColons(mac_add).c_str());
  DEBUG_PRINTF("size of incoming data is : %d, and size of struct is %d \n",  data_len, sizeof(SensorPayloadTemp));

  SensorPayloadTemp *payload = (SensorPayloadTemp*) data;

  DEBUG_PRINTF("battery Percentage %d",payload->batteryPercentage);
  DEBUG_PRINTF("Temperatue%.1f",payload->temperature);
  DEBUG_PRINTF("SensorProfile%d",payload->sensorProfile);
  
  
  // check if it is a valid packet from our nfodes, it has to have our magic
  if (payload->magic != ESPNOW_NODEPACKET_MAGIC) {
    DEBUG_PRINTLN("magic byte invalid");
    return;
  }

  if (payload->sensorProfile == FWRequestPayload) {
    DEBUG_PRINTLN("Processing fw request");
    processNodeUpdateRequest(mac_add, payload);
    return;
  }

  deviceState.enqueSensorPayload(convertToStringWithoutColons(mac_add).c_str(), payload,timeClient);
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
