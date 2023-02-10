#include <Wire.h>
#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "cloudInteractGSM.h"
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
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>
#include "PubSubClient.h"

SCD4x scd_sensor;

Ticker sensorCheckTimer;
DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);
CloudTalk cloudTalk;

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

CloudTalkGSM cloudTalkGsm;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
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
  if (((int)rtc_get_reset_reason(0) == 12) || ((int)rtc_get_reset_reason(0) == 1))
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
#ifdef OLED_DISPLAY
    clearDisplay();
    RSTATE.displayEvents = DisplayPortalConfig;
    drawDisplay(RSTATE.displayEvents);
#endif
  }
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MODEM_PWKEY, OUTPUT);
  modemPowerKeyToggle();
  cloudTalkGsm.restartModem(&modem);
  cloudTalkGsm.initialiseModem(&modem);
  cloudTalkGsm.updateNTPTime(&modem);
  cloudTalkGsm.configureSSL(&modem);
  if (!cloudTalkGsm.startMQTTService(&modem))
  {
    DEBUG_PRINTLN("Error Starting MQTT service");
  }
  if (!cloudTalkGsm.accquireClient(&modem))
  {
    DEBUG_PRINTLN("Error Accquiring client");
  }
  if (!cloudTalkGsm.connectMQTT(&modem))
  {
    DEBUG_PRINTLN("ERROR Connecting to MQTT");
  }

  sensorCheckTimer.attach(1, oneSecCallback);
}

void loop()
{
  if (!RSTATE.isPortalActive)
  {
    if (!isDesiredWiFiAvailable(PSTATE.apSSID) && !RSTATE.isSwitchToGSMRequired)
    {
      DEBUG_PRINTLN("WiFi not available Switch to GSM");
      RSTATE.isSwitchToGSMRequired = true;
    }

    if (!RSTATE.isSwitchToGSMRequired && !reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
    {
      DEBUG_PRINTLN("Error Connecting to WiFi, or switched to GSM");
    }
    
  }

  if (millis() - RSTATE.startPortal >= SECS_PORTAL_WAIT * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
  {
    captivePortal.endPortal();
    RSTATE.isPortalActive = false;
  }

  if (RSTATE.isReadSensorTimeout)
  {
    if (!isSHTAvailable())
    {
      DEBUG_PRINTLN("SHT Not connected, initialising again");
      shtInit();
    }
    else
    {
      readSHT();
      DEBUG_PRINTF("Temperature Value: %1f, Humidity Value: %1f", RSTATE.temperature, RSTATE.humidity);
    }
    if (!isSCDAvailable())
    {
      DEBUG_PRINTLN("SCD Not connected, initialising again");
      scdInit(&scd_sensor);
    }
    else
    {
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

  if (RSTATE.isMqttConnectionTimeout){
      mqtt_check_connection(RSTATE.isSwitchToGSMRequired);
      RSTATE.isMqttConnectionTimeout = false;
  }

  if (RSTATE.isPayloadPostTimeout)
  {
    if (RSTATE.isSwitchToGSMRequired) {
    DEBUG_PRINTLN("Post to cloud  ");
    sensorCheckTimer.detach();
    
      int mqtt_pub_failed_device_event = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);
      int mqtt_start_failed_device_event = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
      int mqtt_connect_failed_device_event = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      int mqtt_subscribe_failed_device_event  = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SubscribeFailed);
      

      DEBUG_PRINTF("failed mqtt events publish failed: %d  start failed: %d, connect failed: %d\n",
                   mqtt_pub_failed_device_event,
                   mqtt_start_failed_device_event,
                   mqtt_connect_failed_device_event);
      if (!mqtt_pub_failed_device_event && !mqtt_connect_failed_device_event)
      {
        DEBUG_PRINTLN("client connected. Publish your meesage");
        cloudTalkGsm.setMQTTTopic(&modem);
        cloudTalkGsm.createMQTTPayload(&modem);
        cloudTalkGsm.publishToTopic(&modem);
      }
         
    }
    else
    {
      cloudTalk.sendPayload();
    }
    RSTATE.isPayloadPostTimeout = false;
    DEBUG_PRINTLN(RSTATE.deviceEvents);
    sensorCheckTimer.attach(1, oneSecCallback);
  }
    mqtt_subscribe_task();
}

void oneSecCallback()
{
  static uint oneSecTick = 0;
  oneSecTick++;

  if (oneSecTick % SENSOR_READINGS_INTERVAL_MSECS == 0)
  {
    RSTATE.isReadSensorTimeout = true;
  }

  if (oneSecTick % PAYLOAD_POST_INTERVAL_MSECS == 0)
  {
    RSTATE.isPayloadPostTimeout = true;
  }

  if (oneSecTick % MQTT_CHECK_CONNECTION == 0)
  {
    RSTATE.isMqttConnectionTimeout = true;
  }



}

void mqtt_subscribe_task()  
{
    if (!testBit(RSTATE.deviceEvents,DeviceStateEvent::DSE_ConnectMqttFailed)){
    int isFailed = testBit(RSTATE.deviceEvents,DeviceStateEvent::DSE_SubscribeFailed); 
        if(isFailed){
            
            cloudTalkGsm.subscribeToTopic(&modem,1);
        }
        if (SerialAT.available()){
            String received = SerialAT.readStringUntil('}');
            Serial.println(received);
  }
 }
}

void mqtt_check_connection(bool isGSMRequired){
  static int connectionRetries = 0;
  if(!isGSMRequired) return;
  int mqtt_pub_failed_device_event = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);
  int mqtt_connect_failed_device_event = testBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
  if (mqtt_connect_failed_device_event){
      if(!cloudTalkGsm.connectMQTT(&modem)){
        if(connectionRetries >4){
          cloudTalkGsm.restartModem(&modem);
        }
        sensorCheckTimer.detach();
        cloudTalkGsm.openNetwork(&modem);
        cloudTalkGsm.releaseMQTTClient(&modem);
        cloudTalkGsm.disconnectMQTTClient(&modem);
        cloudTalkGsm.stopMQTTClient(&modem);
        cloudTalkGsm.updateNTPTime(&modem);
        cloudTalkGsm.configureSSL(&modem);
        cloudTalkGsm.startMQTTService(&modem);
        cloudTalkGsm.accquireClient(&modem);
        cloudTalkGsm.connectMQTT(&modem);
        cloudTalkGsm.subscribeToTopic(&modem,1);
        sensorCheckTimer.attach(1,oneSecCallback);
        connectionRetries++;
      }
  }
}
  

