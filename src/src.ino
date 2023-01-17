#include <Wire.h>
#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "hardwareDefs.h"
#include "sensorRead.h"
#include "utils.h"
#include "WiFiOTA.h"
#include <Ticker.h>
#include <rom/rtc.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include "oledState.h"
#include "SparkFun_SCD4x_Arduino_Library.h"

SCD4x scd_sensor;


Ticker sensorCheckTimer;
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

  delay(500);

  shtInit();
  scdInit(&scd_sensor);

#ifdef OLED_DISPLAY
  initDisplay();
  clearDisplay();
  RSTATE.displayEvents = DisplayCenterTextLogo;
  drawDisplay(RSTATE.displayEvents);
  delay(2000);
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
#ifdef OLED_DISPLAY
    clearDisplay();
    RSTATE.displayEvents = DisplayPortalConfig;
    drawDisplay(RSTATE.displayEvents);
#endif
  }


  pinMode(SIG_PIN, OUTPUT);
  pinMode(CONFIG_PIN, INPUT);


  sensorCheckTimer.attach(1, oneSecCallback);

}

void loop()
{
  if (!RSTATE.isPortalActive)
  {

    if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
    {
      DEBUG_PRINTLN("Error Connecting to WiFi");
    }

  }

  if (millis() - RSTATE.startPortal >= SECS_PORTAL_WAIT * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
  {
    RSTATE.isPortalActive = false;
  }

  if (RSTATE.isReadSensorTimeout) {
    if (!isSHTAvailable()) {
      DEBUG_PRINTLN("SHT Not connected, initialising again");
      shtInit();
    } else {
      readSHT();
      DEBUG_PRINTF("Temperature Value: %1f, Humidity Value: %1f", RSTATE.temperature, RSTATE.humidity);
    }
    if (!isSCDAvailable()) {
      DEBUG_PRINTLN("SCD Not connected, initialising again");
      scdInit(&scd_sensor);
    } else {
      readSCD(&scd_sensor);
      DEBUG_PRINTF("CO2 Value: %1d", RSTATE.carbon);
    }

#ifdef OLED_DISPLAY
    clearDisplay();
    RSTATE.displayEvents = DisplayTempHumiCO2;
    drawDisplay(RSTATE.displayEvents);
#endif
    RSTATE.isReadSensorTimeout = false;
  }

  if (RSTATE.isPayloadPostTimeout) {
    if (cloudTalk.sendPayload())
      blinkLed();
    RSTATE.isPayloadPostTimeout = false;
  }


}

void blinkLed()
{
  digitalWrite(SIG_PIN, HIGH);
  delay(500);
  digitalWrite(SIG_PIN, LOW);
}

void oneSecCallback()
{
  static uint oneSecTick = 0;
  oneSecTick++;
  if (oneSecTick % SENSOR_READINGS_INTERVAL_SECS == 0) {
    RSTATE.isReadSensorTimeout = true;
  }

  if (oneSecTick % PAYLOAD_POST_INTERVAL_SECS == 0) {
    RSTATE.isPayloadPostTimeout = true;
  }
}
