#ifndef HARDWAREDEFS_H
#define HARDWAREDEFS_H


enum DeviceType {
    DT_Gateway        = 1,
    DT_Node        
};

/**
   @brief:x
   Device State Enum
*/
enum DeviceStateEvent {
    DSE_None            = 0,
    DSE_Charging        = 1,
    DSE_BatLow          = 1 << 1,
    DSE_SHTFaulty       = 1 << 2,
    DSE_CCSFaulty       = 1 << 3,
    DSE_DSBFaulty       = 1 << 4,
    DSE_DisplayFault    = 1 << 5, 
    DSE_DisplayDisconnected = 1<<6
};

enum DisplayMode {
  DisplayNone,
  DisplayTemp,
  DisplayTempHumid,
  DisplayGas,
  DisplayCap,
  DisplayDeviceConfig,
  DisplayCenterTextLogo,
  DisplayTempBMP,
  DisplayTempHumidBMP
};

#define INVALID_TEMP_READING      99
#define INVALID_MOISTURE_READING  -1
#define INVALID_HUMIDITY_READING  -1
#define INVALID_CO2_READING       -1
#define BATTERY_VOL_PIN     36


#define AP_MODE_SSID "Gateway-"
#define AP_MODE_PASS "admin"

String formApSsidName(String deviceId) {
  return String(AP_MODE_SSID + deviceId);
}

/**
  @brief: Enable it for displaying serial
  @todo: get the implementation ready 
*/
#define DEBUG_SERIAL 1 
#define DEVICE_SENSOR_TYPE  SensorGas


#define DEVICE_ID_DEFAULT "00:00"
//ESP NOW configuration
#define ESP_NOW_INIT                            1
#define ESPNOW_CHANNEL                          1

#define QUICK_TEST_DATA                         1

#ifdef QUICK_TEST_DATA
#define DATA_SEND_PERIODICITY_SECS              100    // interval for reading sensor data in gateway 100 
#define PROCESS_DATA_INTERVAL_SECS              20    // interval for processing accumulated data on gateway 20 
#else
#define DATA_SEND_PERIODICITY_SECS              600 // 10 mins
#define PROCESS_DATA_INTERVAL_SECS              120  // 5 mins
#endif


#define MAX_MODEM_PWR_RETRIES                   5
#define MAX_WIFI_CONN_RETRIES                   3

//Comment out to disable display
#define OLED_DISPLAY 1


//batery max and min voltages
#define BATT_VOL_0                 3.0
#define BATT_VOL_100               4.2

#define HW_REV                          1 // poc using esp 12e/esp32, no lte modem

// firmware rev
#define FW_REV                          1


//Display Configuration
#define SCREEN_WIDTH                            128 // OLED display width, in pixels
#define SCREEN_HEIGHT                           64 // OLED display height, in pixels


#define MILLI_SECS_MULTIPLIER                   1000
#define MICRO_SECS_MULITPLIER                   1000000
#define SECS_MULTIPLIER_DEEPSLEEP               900 //900
#define SECS_PORTAL_WAIT                        60 //120
#define HTTP_CONNEC_TIMEOUT_IN_MS               100

#define EEPROM_STORE_SIZE                       512
#define EEPROM_STORAGE_FORMAT_VERSION           "c1"
#define EEPROM_STARTING_ADDRESS                 0

#define WAN_WIFI_SSID_DEFAULT                    "Sarthak"
#define WAN_WIFI_PASS_DEFAULT                    "wireless18"


#define BATTERY_INITIAL_READING     0

#define MINIMUM_DEBOUNCE_TIME         250


#ifdef DEBUG_SERIAL
#define DEBUG_PRINTF(...)           Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...)          Serial.println(__VA_ARGS__)
#define DEBUG_PRINT(...)            Serial.print(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINT(...)
#endif


#endif // HARDWAREDEFS_H
