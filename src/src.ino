#include <Wire.h>
#include <ThinghzArduino.h>
#include "hardwareDefs.h"
#include "deviceState.h"
#include "captivePortal.h"
#include "cloudInteract.h"
#include "sensorRead.h"
#include "utils.h"
#include "WiFiOTA.h"
#include <Ticker.h>
#include <rom/rtc.h>
#include <driver/adc.h>
#include "tftState.h"

#define SerialAT Serial1
  
Ticker sensorCheckTimer;
DeviceState state;
DeviceState &deviceState = state;
ESPCaptivePortal captivePortal(deviceState);

#ifdef THINGHZ_ARDUINO_USE_MODEM
  TinyGsm modem(SerialAT);
#endif

CloudTalk cloudTalk;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  DEBUG_PRINTLN("This is THingHz Smart Tissue Culture Rack");
  if (!EEPROM.begin(EEPROM_STORE_SIZE))
  {
    DEBUG_PRINTLN("Problem loading EEPROM");
  }

   if (!SPIFFS.begin(true))
  {
    Serial.println("spiffs mount failed");
    return;
  } 
  else
  {
    Serial.println("spiffs mount success");
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
  lightInit();

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

/*#ifdef OLED_DISPLAY
    clearDisplay();
    RSTATE.displayEvents = DisplayPortalConfig;
    drawDisplay(RSTATE.displayEvents);
#endif*/
  }

  #ifdef THINGHZ_ARDUINO_USE_MODEM
    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_FLIGHT, OUTPUT);
  #endif

  pinMode(SIG_PIN, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);

  digitalWrite(RELAY_PIN_1,PSTATE.light_state_1);
  digitalWrite(RELAY_PIN_2,PSTATE.light_state_2);
  digitalWrite(RELAY_PIN_3,PSTATE.light_state_2);
  digitalWrite(RELAY_PIN_4,PSTATE.light_state_2);
  
  #ifdef THINGHZ_ARDUINO_USE_MODEM
    modemPowerKeyToggle();
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    initialiseModem(&modem); //orwe can call init
    checkNetwork(&modem);
    if(checkNetwork(&modem)){
        connectGPRS(&modem);
    }
  #endif
  
  sensorCheckTimer.attach(1, oneSecCallback);
  Thinghz.sub_topic = MQTT_TOPIC_SUBSCRIBE;
}

void loop()
{
  if (!RSTATE.isPortalActive)
  {
    #ifdef THINGHZ_ARDUINO_USE_MODEM
      if(!checkNetwork(&modem) && !connectGPRS(&modem)){
          DEBUG_PRINTLN("Error connecting to network");
      }else{
          RSTATE.isNetworkConnected = true;     
      }
    #else
      if (!reconnectWiFi((PSTATE.apSSID).c_str(), (PSTATE.apPass).c_str(), 300))
        {
          DEBUG_PRINTLN("Error connecting to WiFi, Trying again");
        }else{
            RSTATE.isNetworkConnected = true;
        }
    #endif
     if (RSTATE.isNetworkConnected && !RSTATE.isThinghzBegin) {

      #ifdef THINGHZ_ARDUINO_USE_MODEM  
        if (!Thinghz.begin(&modem))
        {
          DEBUG_PRINTLN("Thinghz Client Initialization Failed.");
        }
        else
        {
          Thinghz.setActionCallback(ThinghzActionsCallback);
          DEBUG_PRINTLN("Thinghz Client is Initialized Successfully.");
          RSTATE.isThinghzBegin = true;
        }
      #else
        if (!Thinghz.begin())
        {
          DEBUG_PRINTLN("Thinghz Client Initialization Failed.");
        }
        else
        {
          Thinghz.setActionCallback(ThinghzActionsCallback);
          DEBUG_PRINTLN("Thinghz Client is Initialized Successfully.");
          RSTATE.isThinghzBegin = true;
        }
      #endif
    } 

  Thinghz.loop();
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
      DEBUG_PRINTF("Temperature Value: %1f, Humidity Value: %1f\n", RSTATE.temperature, RSTATE.humidity);
    }
    if (!isLightAvailable())
    {
      DEBUG_PRINTLN("BH1750 Not connected, initialising again");
      lightInit();
    }
    else
    {
      readLight();
      DEBUG_PRINTF("Lux Value: %.1f\n", RSTATE.lux);
    }

#ifdef OLED_DISPLAY
    clearDisplay();
    RSTATE.displayEvents = DisplayTempHumiLux;
    drawDisplay(RSTATE.displayEvents);
#endif
    RSTATE.isReadSensorTimeout = false;
  }

  if (millis() - RSTATE.startPortal >= SECS_PORTAL_WAIT * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
  {
    captivePortal.endPortal();
    RSTATE.isPortalActive = false;
  }

  if (RSTATE.isPayloadPostTimeout)
  {
     DEBUG_PRINTLN("Post to cloud  ");
      sensorCheckTimer.detach();
      String payload = cloudTalk.createPayload(DEVICE_SENSOR_TYPE);
      if(!Thinghz.publishToIoT(payload.c_str(),MQTT_TOPIC_PUBLISH)){
          DEBUG_PRINTLN("Publish Failed");
      }
      blinkSignalLed(HIGH);
      RSTATE.isPayloadPostTimeout = false;
      DEBUG_PRINTLN(RSTATE.deviceEvents);
      deviceState.store();
      sensorCheckTimer.attach(1, oneSecCallback);
  }
}

void oneSecCallback()
{
  static uint oneSecTick = 0;
  oneSecTick++;

  if (oneSecTick % SENSOR_READINGS_INTERVAL_S == 0)
  {
    RSTATE.isReadSensorTimeout = true;
  }

  if (oneSecTick % PAYLOAD_POST_INTERVAL_S == 0)
  {
    RSTATE.isPayloadPostTimeout = true;
  }

  if (oneSecTick % MQTT_CHECK_CONNECTION_INTERVAL_S == 0)
  {
    RSTATE.isMqttConnectionTimeout = true;
  }
}

void ThinghzActionsCallback(char* topic, byte* payload, unsigned int length) {
  sensorCheckTimer.detach();
  DEBUG_PRINT("Action received");
  //clearDisplay();
  RSTATE.displayEvents = DisplayEventActionReceived;
  drawDisplay(RSTATE.displayEvents);
  int strSize = 0;
  int tempVar = 0;

  strSize = length + 1;

  if(strSize <= 1) {
    DEBUG_PRINTLN("failed to get action json size");
    return;
  }

  char* actionJsonStr = (char*) malloc(strSize);
  if (actionJsonStr == NULL) {
      DEBUG_PRINTLN("Failed To allcate memory for action json");
    return;
  }

  tempVar = snprintf(actionJsonStr, strSize, "%s", (char*)payload);

  cloudTalk.handleSubscribe((char*)actionJsonStr);
  String mqtt_ack_topic = cloudTalk.createSubscribeTopic(true);
  if (!RSTATE.light_state_1 || !RSTATE.light_state_2 || !RSTATE.light_state_3 || !RSTATE.light_state_4){
      Thinghz.publishToIoT("{\"Success\": \"true\", \"Status\": \"on\"}", mqtt_ack_topic.c_str());
  }else{
      Thinghz.publishToIoT("{\"Success\": \"true\", \"Status\": \"false\"}", mqtt_ack_topic.c_str());
  } 
  sensorCheckTimer.attach(1, oneSecCallback);
  blinkSignalLed(HIGH);
  free(actionJsonStr);
}





