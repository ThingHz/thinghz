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
const char* deviceTypeEnumToString(uint8_t devType) {
  static const char* const map[] = { "ThingHz_Standalone", "ThingHz_EspNow_Node",
                                     "ThingHz_EspNow_WiFiLteHaul_Gateway", "ThingHz_CC1100_Node",
                                     "ThingHz_CC1100_WiFiLTEHaul_Gateway", "DeviceTypeUnknown"
                                   };
  return map[devType];
}

#define AP_MODE_SSID "ThingHz-"



String formApSsidName(String deviceId) {
  return String(AP_MODE_SSID + deviceId);
}

//SPIFFS path for OTA firmware
#define SPIFF_OTA_PATH "/"

/**
 * @brief 
 * SIM800L modem pins 
*/

#define MODEM_PWKEY          4
#define MODEM_TX             26
#define MODEM_RX             27
#define MODEM_FLIGHT         25


//Comment out to disable debugging constraint
//#define DEBUG 1

//Comment to disable serial dump
#define DEBUG_SERIAL    1 
#define SerialAT        Serial1 //define Serial 1 for TinyGSM Serial

//Comment out to disable display
#define OLED_DISPLAY 1

//batery max and min voltages
#define BATT_VOL_0                 3.0
#define BATT_VOL_100               4.2


//Enum for Device Sensor profile
#define DEVICE_SENSOR_TYPE  SensorLight

// hardware rev is tied to device type, they both form a combo that decies the firmware behaviour
#define HW_REV                          1 

// firmware rev
#define FW_REV                          1

//missied Data point spiff file
#define MISS_POINT_STORE_FILE_NAME      "/missDataPoint.txt"

/**
 * @brief 
 * Wiring configuration pin 
*/
#define RELAY_PIN             15                   //Relay pin
#define RELAY_PIN_1           5
#define SIG_PIN               2                   //Status Signal pin


/**
 * @brief 
 * SSD1306 Screen Width 
*/
#define SCREEN_WIDTH                            128 // OLED display width, in pixels
#define SCREEN_HEIGHT                           64 // OLED display height, in pixels


/**
 * @brief 
 * Millis and Micro multipliers 
*/
#define MILLI_SECS_MULTIPLIER                   1000
#define MICRO_SECS_MULITPLIER                   1000000

// deepsleep seconds
#define SECS_MULTIPLIER_DEEPSLEEP               900 

//captive portal wait seconds
#define SECS_PORTAL_WAIT                        60 

//http connection timeout in milli seconds
#define HTTP_CONNEC_TIMEOUT_IN_MS               100

//Sensor reading interval in seconds
#define SENSOR_READINGS_INTERVAL_S              5

#ifndef DEBUG
//payload post interval in seconds 120:2mins 300:5mins 600:10mins 
#define PAYLOAD_POST_INTERVAL_S                 300
#else
//payload post interval in seconds 120:2mins 300:5mins 600:10mins (case of debug) 
#define PAYLOAD_POST_INTERVAL_S                 30
#endif

#define MAX_GSM_RETRIES                         3


//mqtt check connection in seconds 120:2mins 300:5mins 600:10mins 
#define MQTT_CHECK_CONNECTION_INTERVAL_S        20



//AT wait response time
#define AT_WAIT_TIME_MSECS                      12000L


/**
 * @brief 
 * Certificates file name
 */
#define CLIENTCERT_FILE_NAME                    "clientcert.pem"
#define CACERT_FILE_NAME                        "cacert.pem"
#define CLIENTKEY_FILE_NAME                     "clientkey.pem"

/**
 * @brief 
 * MQTT Constants
 */
#define MQTT_HOST                               "tcp://a26dm966t9g0lv-ats.iot.us-east-1.amazonaws.com:8883"      
#define MQTT_HOST_USING_PUBSUB                  "a26dm966t9g0lv-ats.iot.us-east-1.amazonaws.com"
#define MQTT_TOPIC                              "aws/thing/thinghz/"
#define NTP_SERVER                              "pool.ntp.org"
#define MQTT_CLIENT_NAME                        "thinghz_cleint01"

/**
 * @brief 
 * EEPROM preprocessors
 */

#define EEPROM_STORE_SIZE                       512   //EEPROM Size
#define EEPROM_STORAGE_FORMAT_VERSION           "c1"  //EEPROM storage version just a char to match
#define EEPROM_STARTING_ADDRESS                 0     //EEPROM starting address

/**
 * @brief 
 * WiFi Configuration default preprocessors
 */
#define WAN_WIFI_SSID_DEFAULT                    "TimTim"     
#define WAN_WIFI_PASS_DEFAULT                    "wireless18"  

//ThingHz default Device Id
#define DEVICE_ID_DEFAULT                        "THING00001"





/**
 * @brief 
 * Battery  default preprocessors
 */
#define BATTERY_INITIAL_READING     0
#define BATTERY_FINAL_READING       100

/**
 * @brief 
 * Invalid readings default preprocessors
 */
#define INVALID_TEMP_READING        99
#define INVALID_HUMIDITY_READING    0
#define INVALID_GAS_READING        -1
#define INVALID_LIGHT_READING      -1
#define INVALID_MOISTURE_READING   -1
#define INVALID_CAP_READING        -1
#define INVALID_GYRO_READING       -1
#define INVALID_ACCEL_READING      -1
#define INVALID_ALTITUDE_READING   -1
#define INVALID_SEA_READING        -1
#define INVALID_BMP_TEMP_READING    99
#define INVALID_BMP_P_READING      -1
#define INVALID_CO2_READING         0
#define INVALUD_LUX_READING         0
#define DEFAULT_STATE_READING       1
#define DEFAULT_THRESH_READING      0
#define CURRENT_TIME                "0000"

/**
 * @brief 
 * calibration levels
 */
#define CALIBRATION_LEVEL_TEMP      0
#define CALIBRATION_LEVEL_HUMID     0
#define CALIBRATION_LEVEL_CARBON    0

#define ALTITUDE_FOR_SCD            327


/**
 * @brief 
 * escalation ranges for parameters
 */
#define MIN_TARGET_TEMP               4
#define MAX_TARGET_TEMP               20
#define MIN_TARGET_HUMID              20
#define MAX_TARGET_HUMID              60
#define MIN_TARGET_GAS                600
#define MAX_TARGET_GAS                1800


/**
 * @brief 
 * Wakeup count default preprocessors
 */
#define MAX_WAKEUP_COUNT              2
#define MIN_WAKEUP_COUNT              0


/**
 * @brief 
 * dump at commands in serial
 */

#ifdef DEBUG
  #define DUMP_AT_COMMANDS              1
#endif

/**
 * @brief 
 * DBUG_SERIAL preprocessors
 */
#ifdef DEBUG_SERIAL
#define DEBUG_PRINTF(...)           Serial.printf(__VA_ARGS__)
#define DEBUG_PRINTLN(...)          Serial.println(__VA_ARGS__)
#define DEBUG_PRINT(...)            Serial.print(__VA_ARGS__)
#define DEBUG_WRITE(...)            Serial.write(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINT(...)
#define DEBUG_WRITE(...)            
#endif


#endif // HARDWAREDEFS_H
