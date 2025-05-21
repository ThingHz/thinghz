#include "DeviceState.h"

// Constructor for RunTimeState
RunTimeState::RunTimeState() :
    deviceEvents(DeviceStateEvent::DSE_None),
    displayEvents(DisplayMode::DisplayNone),
    isNetworkConnected(false),
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
    light_state_1(DEFAULT_STATE_READING),
    light_state_2(DEFAULT_STATE_READING),
    light_state_3(DEFAULT_STATE_READING),
    light_state_4(DEFAULT_STATE_READING),
    light_thresh(DEFAULT_THRESH_READING),
    isSwitchToGSMRequired(false),
    isReadSensorTimeout(false),
    isPayloadPostTimeout(false),
    isMqttConnectionTimeout(false),
    isMQTTConnected(false),
    isNetworkActive(false),
    gsmConnectionRetries(MAX_GSM_RETRIES),
    gsm_time(CURRENT_TIME)
{
}

// Constructor for PersistantState
PersistantState::PersistantState() : apSSID(WAN_WIFI_SSID_DEFAULT),
    apPass(WAN_WIFI_PASS_DEFAULT),
    apn(DEVICE_ID_DEFAULT),
    tempCalibration(CALIBRATION_LEVEL_TEMP),
    humidCalibration(CALIBRATION_LEVEL_HUMID),
    lightCalibration(CALIBRATION_LEVEL_CARBON),
    isOtaAvailable(0),
    newfWVersion(0),
    light_state_1(0),
    light_state_2(0),
    light_state_3(0),
    light_state_4(0)
{
}

// Equality operator for PersistantState
bool PersistantState::operator==(const PersistantState& rhs) {
    return ((apSSID == rhs.apSSID) &&
            (apPass == rhs.apPass) &&
            (apn == rhs.apn) &&
            (tempCalibration == rhs.tempCalibration) &&
            (humidCalibration == rhs.humidCalibration) &&
            (lightCalibration == rhs.lightCalibration) &&
            (isOtaAvailable == rhs.isOtaAvailable) &&
            (newfWVersion == rhs.newfWVersion) &&
            (light_state_1 == rhs.light_state_1) &&
            (light_state_2 == rhs.light_state_2) &&
            (light_state_3 == rhs.light_state_3) &&
            (light_state_4 == rhs.light_state_4));
}

// Constructor for PersistantStateStorageFormat
PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState) {
    strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
    strcpy(apSSID, persistantState.apSSID.c_str());
    strcpy(apPass, persistantState.apPass.c_str());
    strcpy(apn, persistantState.apn.c_str());
    tempCalibration = persistantState.tempCalibration;
    humidCalibration = persistantState.humidCalibration;
    lightCalibration = persistantState.lightCalibration;
    isOtaAvailable = persistantState.isOtaAvailable;
    newfWVersion = persistantState.newfWVersion;
    light_state_1 = persistantState.light_state_1;
    light_state_2 = persistantState.light_state_2;
    light_state_3 = persistantState.light_state_3;
    light_state_4 = persistantState.light_state_4;
}

// Constructor for PersistantState from storage format
PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore) {
    apSSID = String(persistantStore.apSSID);
    apPass = String(persistantStore.apPass);
    apn = String(persistantStore.apn);
    tempCalibration = persistantStore.tempCalibration;
    humidCalibration = persistantStore.humidCalibration;
    lightCalibration = persistantStore.lightCalibration;
    isOtaAvailable = persistantStore.isOtaAvailable;
    newfWVersion = persistantStore.newfWVersion;
    light_state_1 = persistantStore.light_state_1;
    light_state_2 = persistantStore.light_state_2;
    light_state_3 = persistantStore.light_state_3;
    light_state_4 = persistantStore.light_state_4;
}

// Constructor for DeviceState
DeviceState::DeviceState() {
    // EEPROM.begin(EEPROM_STORE_SIZE); // Uncomment if needed
}

// Destructor for DeviceState
DeviceState::~DeviceState() {
    EEPROM.end();
}

// Store function for DeviceState
bool DeviceState::store() {
    bool retValue = storeEEPROM();
    if (!retValue) {
        DEBUG_PRINTLN("Problem Storing to EEPROM");
        return false;
    }
    return retValue;
}

// Load function for DeviceState
bool DeviceState::load() {
    bool retValue = loadEEPROM();
    if (!retValue) {
        DEBUG_PRINTLN("Problem loading from EEPROM");
        return false;
    }
    return retValue;
}

// Store EEPROM function
bool DeviceState::storeEEPROM() {
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

// Load EEPROM function
bool DeviceState::loadEEPROM() {
    PersistantStateStorageFormat persistantStore;
    EEPROM.get(0, persistantStore);
    if (strcmp(persistantStore.version, EEPROM_STORAGE_FORMAT_VERSION) != 0) {
        DEBUG_PRINTLN("storage format doesn't match, let defaults load, will become proper in next write.");
        return true;
    }
    persistantState = PersistantState(persistantStore);
    eepromRealState = persistantState;
    return true;
}

// Store SPIFF function (currently a placeholder)
bool DeviceState::storeSPIFF() {
    return true;
}
