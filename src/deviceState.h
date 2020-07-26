#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "hardwaredefs.h"
#include "util.h"


/**
 * @brief:
 * Device State Enum   
*/
enum DeviceStateEvent {
  DSE_None                    = 0,
  DSE_SHTDisconnected         = 1,
  DSE_SHTFaulty               = 1 << 1,
  DSE_TMPDisconnected         = 1 << 2,
  DSE_TMPFaulty               = 1 << 3,
  DSE_DSBFaulty               = 1 << 4,
  DSE_TEMPAlarm               = 1 << 5,
  DSE_HUMIDAlarm              = 1 << 6   
};

//advance declaration
class PersistantStateStorageFormat;


/**
 * @brief:
 * Class for runtime Device status
*/
class RunTimeState {
  public:
    RunTimeState() : 
      roomTempTH(INVALID_TEMP_READING), 
      roomTemp(INVALID_TEMP_READING),
      roomHumid(INVALID_HUMIDITY_READING),
      deviceEvents(DeviceStateEvent::DSE_None),
      isWiFiConnected(false),
      isAPActive(false),
      macAddr(DEVICE_ID),
    {
    }

    float roomTempTH;
    float roomTemp;  
    float roomHumid;
    uint deviceEvents;
    bool isWiFiConnected;
    bool isAPActive;
    String macAddr;
    
};

/**
 * @brief:
 * Class EEPROM device format
*/

class PersistantState {
  public:
    PersistantState() : apSSID(WAN_WIFI_SSID_DEFAULT), 
                        apPass(WAN_WIFI_PASS_DEFAULT),
                        targetTemp(MIN_TARGET_TEMP), 
                        targetHumidity(0.0f), 
                        isOtaAvailable(0), 
                        newfWVersion(0),
                        }

    PersistantState(const PersistantStateStorageFormat& persistantStore);

    bool operator==(const PersistantState& rhs) {
      return ((apSSID == rhs.apSSID) &&
              (apPass == rhs.apPass) &&
              (deviceId == rhs.deviceId) &&
              (targetTemp == rhs.targetTemp) &&
              (targetHumidity == rhs.targetHumidity) &&
              (isOtaAvailable == rhs.isOtaAvailable) &&
              (newfWVersion == rhs.newfWVersion) );
    }
    // public data members
    String apSSID;
    String apPass;
    float targetTemp;
    float targetHumidity;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
};

/**
 * @brief:
 * Structure EEPROM Storage format
 * this shadwos persistnat state structure in every way except that
 * it replaces complex data types with POD types, complex data can't be directly stored and
 * read back as is. It was required because we don't want to deal with c strings in rest of the code.
*/

struct PersistantStateStorageFormat {
  public:
    PersistantStateStorageFormat() {}
    PersistantStateStorageFormat(const PersistantState &persistantState);
    char version[8];
    char apSSID[30];
    char apPass[30];
    float setTemp;
    float setHumid;
    uint8_t isOtaAvailable;
    uint8_t newfWVersion;
} __attribute__ ((packed));

PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore)
{
  apSSID = String(persistantStore.apSSID);
  apPass = String(persistantStore.apPass);
  targetTemp = persistantStore.setTemp;
  targetHumidity = persistantStore.setHumid;
  isOtaAvailable = persistantStore.isOtaAvailable;
  newfWVersion = persistantStore.newfWVersion;
}

PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState)
{
  strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
  strcpy(apSSID, persistantState.apSSID.c_str());
  strcpy(apPass, persistantState.apPass.c_str());
  setTemp = persistantState.targetTemp;
  setHumid = persistantState.targetHumidity;
  isOtaAvailable = persistantState.isOtaAvailable;
  newfWVersion = persistantState.newfWVersion;
}

/**
 * @brief:
 * Structure RTC State
 * @todo:
 * We need to switch from EEPROM to RTC
*/

struct RTCState {
  public:
    RTCState() : resetCounter(0), wifiConnFailureCounter(0) {

    }
    uint8_t  resetCounter;                              // do we need to use it for http timouts when lte is not working. both these varialbes need a modem reset beyond a certain point.
    uint8_t  wifiConnFailureCounter;                    // Counter for number of cycles that ended without connection.
} __attribute__ ((packed));

class DeviceState
{
  public:
    // public data members
    RunTimeState        runTimeState;
    PersistantState     persistantState;
    RTCState            rtcState;

    DeviceState() {
    /**
     * @todo:There was a problem in begining it here
    */  
      //EEPROM.begin(EEPROM_STORE_SIZE);
    }
    ~DeviceState() {
      EEPROM.end();
    }

    /**
     * @brief:Load and Store helper functions
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
#define RTCSTATE deviceState.rtcState

#endif // DEVICESTATE_H