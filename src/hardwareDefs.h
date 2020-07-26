#ifndef HARDWAREDEFS_H
#define HARDWAREDEFS_H


enum DeviceType {
  ThingHz_Standalone = 1,                // Sensor Node will work as a standalone device and talks to cloud
  ThingHz_EspNow_Node,                   // Sensor Node will talk back and forth with the Gateway using ESPNow
  ThingHz_EspNow_WiFiLteHaul_Gateway,    // gateway is powered device that comMunicate with Sensor Nodes using ESPNOw Protocol and post the data to cloud
  ThingHz_CC1100_Node,                   // Sensor Node will talk back and forth with the Gateway using CC1100 
  ThingHz_CC1100_WiFiLTEHaul_Gateway,    // gateway is powered device that communicate with Sensor Nodes using CC1100 and post the data to cloud
  DeviceTypeUnknown = 5000
};

// used to convert enum to strings for sending to could in http requests or printing in debug messages
const char* deviceTypeEnumToString(uint8_t devType){
  static const char* const map[] = { "ThingHz_Standalone", "ThingHz_EspNow_Node",
                                     "ThingHz_EspNow_WiFiLteHaul_Gateway", "ThingHz_CC1100_Node",
                                     "ThingHz_CC1100_WiFiLTEHaul_Gateway", "DeviceTypeUnknown"
                                   };
  return map[devType];
}

const char ssidAPConfig[] = "ThingHz";

#define SPIFF_OTA_PATH "/"

#define DEBUG_SERIAL 1 // should come from build system
//#define USE_RTOS 1
#define DUMMY_SHT     1
#define DUMMY_DALLAS  1
#define DUMMY_DISPLAY 1

#define UDP_PACKET_BROADCAST 1

// hardware identifier and rev
#define DEVICE_TYPE     ThingHz_Standalone                    

// hardware rev is tied to device type, they both form a combo that decies the firmware behaviour
#define HW_REV                          1 // poc using esp 12e/esp32, no lte modem

// firmware rev
#define FW_REV                          20


// wiring configuration
#define TEMP_SENSOR_PIN     32
#define BATTERY_VOL_PIN     36

#define MILLI_SECS_MULTIPLIER                   1000
#define MICRO_SECS_MULITPLIER                   1000000


#define EEPROM_STORE_SIZE                       512
#define EEPROM_STORAGE_FORMAT_VERSION           "c1"
#define EEPROM_STARTING_ADDRESS                 0

#define WAN_WIFI_SSID_DEFAULT                    "ThingHz_net"
#define WAN_WIFI_PASS_DEFAULT                    "ThingHz123"

#define NODE_CAPTIVE_PORTAL_MAX_CONFIG_TIME_SECS  120

#define INVALID_TEMP_READING      99
#define INVALID_HUMIDITY_READING  -1

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