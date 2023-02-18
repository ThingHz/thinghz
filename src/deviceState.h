#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "hardwareDefs.h"

/**
   @brief:
   Device State Enum
*/
enum DeviceStateEvent {
  DSE_None                    = 0,
  DSE_SHTDisconnected         = 1,
  DSE_SHTFaulty               = 1 << 1,
  DSE_GASFaulty               = 1 << 7,
  DSE_LIGHTFaulty             = 1 << 9,
  DSE_DisplayDisconnected     = 1 << 10,
  DSE_SimStatusZero           = 1 << 11,
  DSE_ConnectMqttFailed       = 1 << 12,
  DSE_MessagePublishFailed    = 1 << 13,
  DSE_StartMqttFailed         = 1 << 14,
  DSE_SubscribeFailed         = 1 << 15,
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
  DisplayPortalConfig
};

//advance declaration
class PersistantStateStorageFormat;


/**
   @brief:
   Class for runtime Device status
*/
class RunTimeState {
  public:
    RunTimeState():
      deviceEvents(DeviceStateEvent::DSE_None),
      displayEvents(DisplayMode::DisplayNone),
      isWiFiConnected(false),
      isAPActive(false),
      isPortalActive(false),
      startPortal(0),
      macAddr(DEVICE_ID_DEFAULT),
      batteryPercentage(100),
      temperature(INVALID_TEMP_READING),
      humidity(INVALID_HUMIDITY_READING),
      carbon(INVALID_CO2_READING),
      lux(INVALUD_LUX_READING),
      light_state(DEFAULT_STATE_READING),
      light_thresh(DEFAULT_THRESH_READING),
      isReadSensorTimeout(false),
      isPayloadPostTimeout(false),
      isSwitchToGSMRequired(false),
      isMqttConnectionTimeout(false),
      gsmConnectionRetries(MAX_GSM_RETRIES)
    {

    }

    uint deviceEvents;
    DisplayMode displayEvents;
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
    uint8_t light_state;
    uint light_thresh; 
    bool isReadSensorTimeout;
    bool isPayloadPostTimeout;
    bool isSwitchToGSMRequired;
    bool isMqttConnectionTimeout;
    int gsmConnectionRetries;
};

/**
   @brief:
   Class EEPROM device format
*/

class PersistantState {
  public:
    PersistantState() : apSSID(WAN_WIFI_SSID_DEFAULT),
      apPass(WAN_WIFI_PASS_DEFAULT),
      deviceId(DEVICE_ID_DEFAULT),
      tempCalibration(CALIBRATION_LEVEL_TEMP),
      humidCalibration(CALIBRATION_LEVEL_HUMID),
      carbonCalibration(CALIBRATION_LEVEL_CARBON),
      isOtaAvailable(0),
      newfWVersion(0)
    {

    }

    PersistantState(const PersistantStateStorageFormat& persistantStore);

    bool operator==(const PersistantState& rhs) {
      return ((apSSID == rhs.apSSID) &&
              (apPass == rhs.apPass) &&
              (deviceId == rhs.deviceId) &&
              (tempCalibration == rhs.tempCalibration) &&
              (humidCalibration == rhs.humidCalibration) &&
              (carbonCalibration == rhs.carbonCalibration) &&
              (isOtaAvailable == rhs.isOtaAvailable) &&
              (newfWVersion == rhs.newfWVersion));
    }
    // public data members
    String apSSID;
    String apPass;
    String deviceId;
    int tempCalibration;
    int humidCalibration;
    int carbonCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
   
};

/**
   @brief:
   Structure EEPROM Storage format
   this shadwos persistnat state structure in every way except that
   it replaces complex data types with POD types, complex data can't be directly stored and
   read back as is. It was required because we don't want to deal with c strings in rest of the code.
*/

struct PersistantStateStorageFormat {
  public:
    PersistantStateStorageFormat() {}
    PersistantStateStorageFormat(const PersistantState &persistantState);
    char version[8];
    char apSSID[30];
    char deviceId[30];
    char apPass[30];
    int tempCalibration;
    int humidCalibration;
    int carbonCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
} __attribute__ ((packed));

PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore)
{
  apSSID = String(persistantStore.apSSID);
  apPass = String(persistantStore.apPass);
  deviceId = String(persistantStore.deviceId);
  tempCalibration = persistantStore.tempCalibration ;
  humidCalibration = persistantStore.humidCalibration;
  carbonCalibration = persistantStore.carbonCalibration;
  isOtaAvailable = persistantStore.isOtaAvailable;
  newfWVersion = persistantStore.newfWVersion;
}

PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState)
{
  strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
  strcpy(apSSID, persistantState.apSSID.c_str());
  strcpy(apPass, persistantState.apPass.c_str());
  strcpy(deviceId, persistantState.deviceId.c_str());
  tempCalibration = persistantState.tempCalibration;
  humidCalibration = persistantState.humidCalibration;
  carbonCalibration = persistantState.carbonCalibration;
  isOtaAvailable = persistantState.isOtaAvailable;
  newfWVersion = persistantState.newfWVersion;
}

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

class DeviceState
{
  public:
    // public data members
    RunTimeState        runTimeState;
    PersistantState     persistantState;


    DeviceState() {
      /**
         @todo:There was a problem in begining it here
      */
      //EEPROM.begin(EEPROM_STORE_SIZE);
    }
    ~DeviceState() {
      EEPROM.end();
    }

    /**
       @brief:Load and Store helper functions
    */
    bool store()
    {
      bool retValue = false;
      retValue = storeEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem Storing to EEPROM");
        return false;
      }
      return retValue;
    }

    bool load()
    {
      bool retValue = false;
      retValue = loadEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem loading from EEPROM");
        return false;
      }
      return retValue;
    }

  private:
    PersistantState eepromRealState;

    bool storeEEPROM()
    {
      if (persistantState == eepromRealState) {
        DEBUG_PRINTLN("nothing to write, state hasn't changed since last read/write");
        return true;
      }

      DEBUG_PRINTLN("Writing EEPROM, in memory structure is dirty");
      PersistantStateStorageFormat persistantStore(persistantState);
      EEPROM.put(0, persistantStore);
      EEPROM.commit();
      eepromRealState = persistantState;
      return true;
    }

    bool loadEEPROM() {
      PersistantStateStorageFormat persistantStore;
      EEPROM.get(0, persistantStore);
      if (strcmp(persistantStore.version, EEPROM_STORAGE_FORMAT_VERSION) != 0) {
        DEBUG_PRINTLN("storage format doens't match, let defaults load, will become proper in next write.");
        return true;
      }
      persistantState = PersistantState(persistantStore);
      eepromRealState = persistantState;
      return true;
    }

    bool storeSPIFF()
    {
      return true;
    }

};

extern DeviceState& deviceState;

// just shortening macros
#define RSTATE   deviceState.runTimeState
#define PSTATE   deviceState.persistantState

#endif // DEVICESTATE_H
