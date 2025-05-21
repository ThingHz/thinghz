#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "hardwareDefs.h"

/**
   @brief:
   Device State Enum
*/
enum DeviceStateEvent {
    DSE_None = 0,
    DSE_SHTFaulty = 1,
    DSE_GASFaulty = 1 << 2,
    DSE_LIGHTFaulty = 1 << 3,
    DSE_DisplayDisconnected = 1 << 4,
    DSE_SimStatusZero = 1 << 5,
    DSE_NoNetwork = 1 << 6,
    DSE_ConnectMqttFailed = 1 << 7,
    DSE_MessagePublishFailed = 1 << 8,
    DSE_StartMqttFailed = 1 << 9,
    DSE_SHTDisconnected = 1 << 10,
    DSE_SubscribeFailed = 1 << 11
};

enum DisplayMode {
    DisplayNone,
    DisplayTemp,
    DisplayTempHumid,
    DisplayGas,
    DisplayCap,
    DisplayDeviceConfig,
    DisplayCenterTextLogo,
    DisplayDeviceHealth,
    DisplayDeviceStatus,
    DisplayTempHumiCO2,
    DisplayTempHumiLux,
    DisplayPortalConfig,
    DisplayEventActionReceived
};

// Forward declaration
class PersistantStateStorageFormat;

/**
   @brief:
   Class for runtime Device status
*/
class RunTimeState {
public:
    RunTimeState();

    uint deviceEvents;
    DisplayMode displayEvents;
    bool isNetworkConnected;
    bool isWiFiConnected;
    bool isAPActive;
    bool isPortalActive;
    unsigned long startPortal;
    String macAddr;
    int batteryPercentage;
    float temperature;
    float humidity;
    uint16_t carbon;
    float lux;
    uint8_t light_state_1;
    uint8_t light_state_2;
    uint8_t light_state_3;
    uint8_t light_state_4;
    uint light_thresh; 
    bool isReadSensorTimeout;
    bool isPayloadPostTimeout;
    bool isSwitchToGSMRequired;
    bool isMqttConnectionTimeout;
    bool isMQTTConnected;
    bool isNetworkActive;
    int gsmConnectionRetries;
    String gsm_time;
};

/**
   @brief:
   Class EEPROM device format
*/
class PersistantState {
public:
    PersistantState();
    PersistantState(const PersistantStateStorageFormat& persistantStore);
    bool operator==(const PersistantState& rhs);

    // Public data members
    String apSSID;
    String apPass;
    String apn;
    int tempCalibration;
    int humidCalibration;
    int lightCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
    uint8_t light_state_1;
    uint8_t light_state_2;
    uint8_t light_state_3;
    uint8_t light_state_4;
};

/**
   @brief:
   Structure EEPROM Storage format
*/
struct PersistantStateStorageFormat {
public:
    PersistantStateStorageFormat();
    PersistantStateStorageFormat(const PersistantState &persistantState);
    char version[8];
    char apSSID[30];
    char apn[30];
    char apPass[30];
    int tempCalibration;
    int humidCalibration;
    int lightCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
    uint8_t light_state_1;
    uint8_t light_state_2;
    uint8_t light_state_3;
    uint8_t light_state_4;
} __attribute__ ((packed));

/**
   @brief:
   Structure RTC State
   @todo:
   We need to switch from EEPROM to RTC
*/
typedef struct {
    int isEscalation;
    int missedDataPoint;
    int wakeUpCount;
} RTCState;

RTC_DATA_ATTR RTCState rtcState;

class DeviceState {
public:
    RunTimeState runTimeState;
    PersistantState persistantState;

    DeviceState();
    ~DeviceState();

    bool store();
    bool load();

private:
    PersistantState eepromRealState;

    bool storeEEPROM();
    bool loadEEPROM();
    bool storeSPIFF();
};

extern DeviceState& deviceState;

// Just shortening macros
#define RSTATE   deviceState.runTimeState
#define PSTATE   deviceState.persistantState

#endif // DEVICESTATE_H
