#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "hardwareDefs.h"
#include "utils.h"


/**
   @brief:
   Device State Enum
*/
enum DeviceStateEvent {
  DSE_None                    = 0,
  DSE_SHTDisconnected         = 1,
  DSE_SHTFaulty               = 1 << 1,
  DSE_TMPDisconnected         = 1 << 2,
  DSE_TMPFaulty               = 1 << 3,
  DSE_DSBFaulty               = 1 << 4,
  DSE_TEMPAlarm               = 1 << 5,
  DSE_HUMIDAlarm              = 1 << 6,
  DSE_GASFaulty               = 1 << 7,
  DSE_GYROFaulty              = 1 << 8,
  DSE_LIGHTFaulty             = 1 << 9,
  DSE_CAPfaulty               = 1 << 10
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
      isWiFiConnected(false),
      isAPActive(false),
      isPortalActive(false),
      startPortal(0),
      macAddr(DEVICE_ID_DEFAULT),
      batteryPercentage(BATT_VOL_100),
      temperature(INVALID_TEMP_READING),
      humidity(INVALID_HUMIDITY_READING),
      capacitance(INVALID_CAP_READING)
    {

    }

    uint deviceEvents;
    bool isWiFiConnected;
    bool isAPActive;
    bool isPortalActive;
    unsigned long startPortal;
    String macAddr;
    int batteryPercentage;
    float temperature;
    float humidity;
    float capacitance;
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
      targetTempMin(MIN_TARGET_TEMP),
      targetHumidityMin(MIN_TARGET_HUMID),
      targetTempMax(MAX_TARGET_TEMP),
      targetHumidityMax(MAX_TARGET_HUMID),
      tempCalibration(CALIBRATION_LEVEL),
      isOtaAvailable(0),
      newfWVersion(0),
      isBLE(0)
    {

    }

    PersistantState(const PersistantStateStorageFormat& persistantStore);

    bool operator==(const PersistantState& rhs) {
      return ((apSSID == rhs.apSSID) &&
              (apPass == rhs.apPass) &&
              (deviceId == rhs.deviceId) &&
              (isOtaAvailable == rhs.isOtaAvailable) &&
              (newfWVersion == rhs.newfWVersion) &&
              (isBLE == rhs.isBLE) &&
              (targetTempMin == rhs.targetTempMin) &&
              (targetHumidityMin == rhs.targetHumidityMin) &&
              (targetTempMax == rhs.targetTempMax) &&
              (targetHumidityMax == rhs.targetHumidityMax) &&
              (tempCalibration == rhs.tempCalibration));
    }
    // public data members
    String apSSID;
    String apPass;
    String deviceId;
    int targetTempMin;
    int targetHumidityMin;
    int targetTempMax;
    int targetHumidityMax;
    int tempCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
    uint8_t isBLE;
   
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
    int setTempMin;
    int setHumidityMin;
    int setTempMax;
    int setHumidityMax;
    int tempCalibration;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
} __attribute__ ((packed));

PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore)
{
  apSSID = String(persistantStore.apSSID);
  apPass = String(persistantStore.apPass);
  deviceId = String(persistantStore.deviceId);
  targetTempMin = persistantStore.setTempMin ;
  targetHumidityMin = persistantStore.setHumidityMin;
  targetTempMax = persistantStore.setTempMax;
  targetHumidityMax = persistantStore.setHumidityMax;
  tempCalibration = persistantStore.tempCalibration;
  isOtaAvailable = persistantStore.isOtaAvailable;
  newfWVersion = persistantStore.newfWVersion;
}

PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState)
{
  strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
  strcpy(apSSID, persistantState.apSSID.c_str());
  strcpy(apPass, persistantState.apPass.c_str());
  strcpy(deviceId, persistantState.deviceId.c_str());
  setTempMin = persistantState.targetTempMin;
  setHumidityMin = persistantState.targetHumidityMin;
  setTempMax = persistantState.targetTempMax;
  setHumidityMax = persistantState.targetHumidityMax;
  tempCalibration = persistantState.tempCalibration;
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
