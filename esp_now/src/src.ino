#include <WiFi.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include "sensor_payload.h"
#include "esputils.h"
#include <esp_now.h>
#include <rom/rtc.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "sensorRead.h"
#include "runTime.h"
#include "captivePortal.h"

/**
   @brief:
   Structure RTC State
*/
typedef struct {
    uint32_t wakeupsCount            = 0;
    uint32_t gatewayNotFoundCount    = 0;
    uint8_t sendFailures             = 0;
    uint8_t gatewayValidCount        = 0;
    uint8_t gatewaymacs[3][6]        = {{ 0 }};
    int8_t rssi[3]                   = {0};
} RTCState;

uint8_t remoteMac[] = {0x9c, 0x9c, 0x1f, 0x1d, 0x2b, 0x19};

ESPCaptivePortal captivePortal;
RTC_DATA_ATTR RTCState rtcState;
esp_now_peer_info_t slave;
const esp_now_peer_info_t *peer = &slave;
TaskHandle_t xHandle;
/**
   @brief:
   Class for ESPnow data handling
*/
class EspNowData {
public:
    EspNowData(uint8_t *dptr, uint8_t len)
    {
        length = len;
        data = new uint8_t[len];
        memcpy(data, dptr, len);
    }

    ~EspNowData()
    {
        delete data;
        data = nullptr;
    }

    uint8_t length = 0;
    uint8_t * data = nullptr;
};

SensorPayloadTemp SensorTemperature;
SensorPayloadTH   SensorTempHumid;
SensorPayloadTHCO2 SensorTHG;


int sendStatusCB = 0;
uint8_t invalidMac[MAC_LEN] = { 0 };

uint8_t successfulMAC[MAC_LEN];

void printrtc()
{
    DEBUG_PRINTLN("");

    DEBUG_PRINTF("gatewayNotFoundCount = %d , sendFailures=%d, gatewayValidCount=%d, ", rtcState.gatewayNotFoundCount, rtcState.sendFailures, rtcState.gatewayValidCount);
    for(int i = 0; i < rtcState.gatewayValidCount; i++)
    {
        printmac( rtcState.gatewaymacs[i]);
        DEBUG_PRINTF("Rssi of %d device = %d", i,rtcState.rssi[i] );
        DEBUG_PRINTLN("");
    }
}

void printmac(uint8_t *mac)
{
    DEBUG_PRINTF("value of first mac is %x:%x:%x:%x:%x:%x \n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    if(mac[0]==0&&mac[1]==0&&mac[2]==0&&mac[3]==0&&mac[4]==0&&mac[5]==0)
    {
        DEBUG_PRINTLN("INvalid Mac");
        rtcState.gatewayValidCount=0;
    }
}




//Step 1- initialise ESPNOW
void initESP(){
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    DEBUG_PRINTLN("Initialization Successful");
  }
  else {
    DEBUG_PRINTLN("Initialization failed");
    ESP.restart();
  }
}

//Step 2 Scan for the available peers

bool scanPeeers(){
    
    rtcState.gatewayValidCount = 0;
    DEBUG_PRINTLN("Entered scan peer");

    int numNetwork =  WiFi.scanNetworks();

    for (int i=0;i<numNetwork;i++) {
        String ssid = WiFi.SSID(i);
        String bssid = WiFi.BSSIDstr(i);
        if(ssid.indexOf("Gateway-")>=0) {
            rtcState.rssi[i] = WiFi.RSSI(i);
            //DEBUG_PRINTF("Gateway info : index=%d, ssid = %s, rssi = %s, bssid = %s\n", i, ssid.c_str(), rtcState.rssi[i], bssid.c_str());
            memcpy(rtcState.gatewaymacs[i], WiFi.BSSID(i), 6);
            rtcState.gatewayValidCount++;
            DEBUG_PRINTLN("rtcState.gatewayValidCount++");
            printmac(rtcState.gatewaymacs[i]);
            /*int mac[6];
            if ( 6 == sscanf(bssid.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
                for (int j = 0; j < 6; ++j ) {
                slave.peer_addr[j] = (uint8_t) mac[j];
            }
          }*/
          slave.channel = ESPNOW_CHANNEL; // pick a channel
          slave.encrypt = 0; // no encryption
        }
    }

    if (rtcState.gatewayValidCount == 0) {
        DEBUG_PRINTLN("No gatewayNotFoundCount Found, trying again.");
        rtcState.gatewayNotFoundCount++;
    }else{
        rtcState.gatewayNotFoundCount = 0;
      }

    // clean up ram
    WiFi.scanDelete();
}


void configureSlave(){
    memcpy( &slave.peer_addr, &remoteMac, 6 );
     slave.channel = ESPNOW_CHANNEL; // pick a channel
     slave.encrypt = 0; // no encryption
  }


//Step 3 Scan for the available peers
bool addPeers(){
    DEBUG_PRINTLN("Add Peers Started ");

        /*int exists = esp_now_is_peer_exist(rtcState.gatewaymacs[i]);    

        if (exists) {
            // Slave already paired.
            DEBUG_PRINTLN("Already Paired");
            continue;
        }*/
        // Slave not paired, attempt pair
        esp_err_t addStatus = esp_now_add_peer(peer);
        if(addStatus == ESP_OK){
            DEBUG_PRINTF("Peer Added");
          }
}


void deletePeer() {
  esp_err_t delStatus = esp_now_del_peer(slave.peer_addr);
  Serial.print("Slave Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_INIT) {
    // How did we get so far!!
    Serial.println("ESPNOW Not Init");
  } else if (delStatus == ESP_ERR_ESPNOW_ARG) {
    Serial.println("Invalid Argument");
  } else if (delStatus == ESP_ERR_ESPNOW_NOT_FOUND) {
    Serial.println("Peer not found.");
  } else {
    Serial.println("Not sure what happened");
  }
}





void storeAndSleep(int sleepSecs)
{
    WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(VOLTAGE_DIV_PIN, HIGH);
  DEBUG_PRINTLN("going to sleep");
  esp_sleep_enable_timer_wakeup(SECS_MULTIPLIER_DEEPSLEEP * MICRO_SECS_MULITPLIER);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_start();
}

void yieldingDelay(uint32_t delayms, uint32_t stepSizems)
{
    uint32_t loopCount = delayms/stepSizems;
    for ( int i = 0; i < loopCount; i++) {
        yield();
        delay(1);
    }
}

void readSensorTemp(){
    DSB112Init();
    readDSB112();
    SensorTemperature.temperature = RSTATE.temperature;
    SensorTemperature.batteryPercentage = getBatteryPercentage(readBatValue());
    SensorTemperature.rssi  = getRSSIasQuality();
    SensorTemperature.deviceEvent = RSTATE.deviceEvents;
}
void readSensorTH(){
     shtInit();
     if (!isSHTAvailable())
          {
            DEBUG_PRINTLN("SHT Not connected");
          }
    readSHT();
    SensorTempHumid.temperature = RSTATE.temperature;
    SensorTempHumid.humidity = RSTATE.humidity;
    SensorTempHumid.batteryPercentage = getBatteryPercentage(readBatValue());
    SensorTempHumid.rssi  = getRSSIasQuality();
    SensorTempHumid.deviceEvent = RSTATE.deviceEvents;
}
void readSensorGas(){
    shtInit();
    if (isSHTAvailable())
          {
            readSHT();
          }

    readCCS();
    SensorTHG.temperature = RSTATE.temperature;
    SensorTHG.humidity = RSTATE.humidity;
    SensorTHG.co2ppm = RSTATE.co2;
    RSTATE.batteryPercentage = getBatteryPercentage(readBatValue());
    if(RSTATE.batteryPercentage <= 10){
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_BatLow);
      }
    SensorTHG.batteryPercentage = RSTATE.batteryPercentage;
    SensorTHG.rssi  = getRSSIasQuality();
    SensorTHG.deviceEvent = RSTATE.deviceEvents;
}

void setup() {
    Serial.begin(115200);
    printrtc();
    DEBUG_PRINTF("The reset reason is %d\n", (int)rtc_get_reset_reason(0));
    if ( (int)rtc_get_reset_reason(0) == 1)  { 
    RSTATE.isPortalActive  = true;
    if (!reconnectAP(AP_MODE_SSID)) {
      DEBUG_PRINTLN("Error Setting Up AP Connection");
      return;
    }
    delay(100);
    captivePortal.beginServer();
    delay(100);
  }

    
    pinMode(SIG_PIN, OUTPUT);
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(CONFIG_PIN, INPUT);
    pinMode(VOLTAGE_DIV_PIN, OUTPUT);
    digitalWrite(VOLTAGE_DIV_PIN, LOW);
    analogSetAttenuation(ADC_0db);
    ccsInit();
    if(isCCSAvailable()){
        DEBUG_PRINTLN("Warming up co2");
        RSTATE.isCO2Available = warmCCS();
      }    
    if(!RSTATE.isPortalActive){
        rtcState.wakeupsCount++; // increment wakeup count.
        WiFi.mode(WIFI_STA);
        DEBUG_PRINTLN("  ");
        DEBUG_PRINTLN("STA MAC: "); DEBUG_PRINTLN(WiFi.macAddress());
        initESP();
        esp_now_register_send_cb(OnDataSent);
    }
    
}


void loop() {

    
    
    if(!RSTATE.isPortalActive){

       if (rtcState.sendFailures>=MAX_DATA_RETRY_COUNT) {
        // give up and go for long sleep, this cycles data will be lost
        rtcState.sendFailures = 0;
        rtcState.gatewayValidCount = 0;
        storeAndSleep(DATA_SEND_PERIODICITY_SECS);
        return;
    }
   
       DEBUG_PRINTLN("processing stored gateways");

    /*if ((rtcState.gatewayValidCount <1)|| (rtcState.gatewayValidCount >3)){
            scanPeeers();
             // give up and go for long sleep, this cycles data will be lost
            if (rtcState.gatewayNotFoundCount>MAX_DATA_RETRY_COUNT) {
                rtcState.gatewayNotFoundCount = 0;
                storeAndSleep(DATA_SEND_PERIODICITY_SECS);
            }
      
      }*/

      configureSlave();
        

    // now we have valid gateways
    addPeers();
    //vTaskSuspend( xHandle );

    int sendStatus = 1; // default assumes failure
    int aggSendStatus = 1;
    const uint8_t *peer_addr = slave.peer_addr;
   //for(int i = 0; i < rtcState.gatewayValidCount; i++)
    //{
        switch (SENSOR_PROFILE)
        {
        case SensorTemp:
            readSensorTemp();
            DEBUG_PRINTF("Temp= %.1f, battery= %d",SensorTemperature.temperature,SensorTemperature.batteryPercentage );
            sendStatus = esp_now_send(peer_addr, (uint8_t*)&SensorTemperature, sizeof(SensorTemperature)); // NULL means send to all peers
            DEBUG_PRINTF("Send status= %d\n",sendStatus );
            break;
        case SensorTH:
            readSensorTH();
            DEBUG_PRINTF("Temp= %.1f, battery= %d, humidity=%.1f",SensorTempHumid.temperature,SensorTempHumid.batteryPercentage,SensorTempHumid.humidity);
            sendStatus = esp_now_send(peer_addr, (uint8_t*)&SensorTempHumid, sizeof(SensorTempHumid)); // NULL means send to all peers
            DEBUG_PRINTF("Send status= %d\n",sendStatus );
            break;
        case SensorGas:
            readSensorGas();
            DEBUG_PRINTF("Temp= %.1f, battery= %d, humidity=%.1f, CO2=%u, deviceEvents=%u",SensorTHG.temperature,SensorTHG.batteryPercentage,SensorTHG.humidity,(unsigned)SensorTHG.co2ppm,SensorTHG.deviceEvent);
            sendStatus = esp_now_send(peer_addr, (uint8_t*)&SensorTHG, sizeof(SensorTHG)); // NULL means send to all peers
            DEBUG_PRINTF("Send status= %d\n",sendStatus );
            break;
        default:
            break;
        }

        
        // wait for sendstatus callback for about 10ms
        yieldingDelay(10, 2);

        // successful
        if (!sendStatus && sendStatusCB)
        {
            aggSendStatus = 0; // mark aggregate success.
           
        }
    //}

    // successful in sending data
    if (!aggSendStatus) {
        DEBUG_PRINTLN("Send status successful" );
        rtcState.sendFailures = 0;
        storeAndSleep(DATA_SEND_PERIODICITY_SECS);
        return;
    }

    rtcState.sendFailures++;

    }

    if (millis() - RSTATE.startPortal >= CAPTIVE_MAX_CLIENT_CONFIG_DURATION * MILLI_SECS_MULTIPLIER && RSTATE.isPortalActive)
    {
        RSTATE.isPortalActive = false;
        WiFi.mode(WIFI_STA);
        DEBUG_PRINTLN("  ");
        DEBUG_PRINTLN("STA MAC: "); DEBUG_PRINTLN(WiFi.macAddress());
        initESP();
        esp_now_register_send_cb(OnDataSent);
    }
}

// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  status == ESP_NOW_SEND_SUCCESS ? sendStatusCB =1 : sendStatusCB=0;
  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



/*void TaskCalculateCCS(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  { 
    ccsInit();
    if(isCCSAvailable()){
        RSTATE.isCO2Available = warmCCS();
      }
      if(RSTATE.isCO2Available){
          readCCS();
        }
        vTaskDelay(100);
  }
  }*/
