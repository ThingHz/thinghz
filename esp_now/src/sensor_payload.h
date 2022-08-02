#ifndef ESPNOWPACKETS_H
#define ESPNOWPACKETS_H

// this file should define all the air packet format for raw payload
// raw payloads will be passed using raw RF like espnow and generic radios
// please ensure that these types remain POD, they should never include any
// dynamic dispatch or pointer arithmatic inside them.

#include "hardwaredefs.h"
#include "assert.h"

#define INVALID_TEMP_READING      99
#define INVALID_MOISTURE_READING  -1
#define INVALID_HUMIDITY_READING  -1
#define INVALID_CO2_READING       -1


enum SensorProfile {
  SensorNone  = 1,
  SensorTemp,
  SensorTH,
  SensorGas,
  
};


enum DeviceStateEvent {
    DSE_None            = 0,
    DSE_Charging        = 1,
    DSE_BatLow          = 1 << 1,
    DSE_SHTFaulty       = 1 << 2,
    DSE_CCSFaulty       = 1 << 3,
    DSE_DSBFaulty       = 1 << 4,
    DSE_DisplayFault    = 1 << 5,  
};

enum DeviceType {
    DT_Gateway        = 1,
    DT_Node        
};


bool testBit(uint &bits, int bit)
{
  return (bits & bit);
}

void setBit(uint &bits, int bit)
{
  bits |= bit;
}

void clearBit(uint &bits, int bit)
{
  bits &= (~bit);
}


struct SensorPayload {
public:
    SensorPayload() :  sensorProfile(SensorProfile::SensorNone), hwRev(HW_REV), fwRev(FW_REV), deviceType(DeviceType::DT_Node),
        rssi(0), batteryPercentage(0),deviceEvent(DeviceStateEvent::DSE_None){ }
    uint8_t sensorProfile;           // identifies sensor profile
    uint8_t hwRev;                  // identifies hw revision
    uint8_t fwRev;                 // identifies fw revision
    uint8_t deviceType;           // identifies device type
    int  rssi;                 // rssi of gateway
    uint8_t batteryPercentage;  //batteryPercentage of nodes
    uint deviceEvent;
}__attribute__ ((packed));

// temprature only definition
struct SensorPayloadTemp : public SensorPayload {
    SensorPayloadTemp() : temperature(INVALID_TEMP_READING) {
        sensorProfile = SensorProfile::SensorTemp;
    }
    float temperature;
}__attribute__ ((packed));


struct SensorPayloadTH : public SensorPayloadTemp {
    SensorPayloadTH() : humidity(INVALID_HUMIDITY_READING) {
        sensorProfile = SensorProfile::SensorTH;
    }
    float humidity;
} __attribute__ ((packed));


struct SensorPayloadTHCO2 : public SensorPayloadTH {
    SensorPayloadTHCO2() : co2ppm(INVALID_CO2_READING) {
        sensorProfile = SensorProfile::SensorGas;
    }
    uint16_t co2ppm;    
} __attribute__ ((packed));




// there might be simpler solution to this. I didn't use virtual destructor because that doesn't solve all the problems
// makes it impossible for these structures to remain pod types.

// caller is responsible for deleting the returned object
//it is just returning the pointer or address of sensor Payload
SensorPayload* copyPayloadObject(const SensorPayload *payload) {
    
    if (!payload) {
        return nullptr;
    }

    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
    {
        SensorPayload *newPayload = new SensorPayload;
        *newPayload = *payload;
        return newPayload;
    }
        break;
    case SensorProfile::SensorTemp:
    {
        SensorPayloadTemp *newPayload = new SensorPayloadTemp;
        SensorPayloadTemp *oldPayload = (SensorPayloadTemp*)payload;
        *newPayload = *oldPayload;
        DEBUG_PRINTF("value of fw from old payload : %d, and new payload is : %d\n", oldPayload->fwRev, newPayload->fwRev);
        return newPayload;
    }
        break;
    case SensorProfile::SensorTH:
    {
        SensorPayloadTH *newPayload = new SensorPayloadTH;
        SensorPayloadTH *oldPayload = (SensorPayloadTH*)payload;
        *newPayload = *oldPayload;
        return newPayload;
    }
        break;
    case SensorProfile::SensorGas:
    {
        SensorPayloadTHCO2 *newPayload = new SensorPayloadTHCO2;
        SensorPayloadTHCO2 *oldPayload = (SensorPayloadTHCO2*)payload;
        *newPayload = *oldPayload;
        return newPayload;
    }
        break;
    default: // should never reach here
        //assert(0);
        DEBUG_PRINTF("Sensor Payload copy called with invalid sensor payload type: %d, we have a null ptr somewhere down the line\n", payload->sensorProfile);
        break;
    }
    return nullptr;
}

void freePayloadObject(SensorPayload *payload) {
    if (!payload) {
        return;
    }

    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
    case SensorProfile::SensorTemp:
    {
        SensorPayloadTemp *oldPayload = (SensorPayloadTemp*)payload;
        delete oldPayload;
    }
        break;
    case SensorProfile::SensorTH:
    {
        SensorPayloadTH *oldPayload = (SensorPayloadTH*)payload;
        delete oldPayload;
    }
        break;
    case SensorProfile::SensorGas:
    {
        SensorPayloadTHCO2 *oldPayload = (SensorPayloadTHCO2*)payload;
        delete oldPayload;
    }
        break;
    default: // should never reach here
        assert(0);
        break;
    }
}

size_t sizeofPaylodObject(const SensorPayload *payload)
{
    switch (payload->sensorProfile) {
    case SensorProfile::SensorNone:
        return sizeof (SensorPayload);
    case SensorProfile::SensorTemp:
        return sizeof (SensorPayloadTemp);
    case SensorProfile::SensorTH:
        return sizeof (SensorPayloadTH);
    case SensorProfile::SensorGas:
        return sizeof (SensorPayloadTHCO2);
    default: // should never reach here
        assert(0);
        break;
    }
}

#endif // ESPNOWPACKETS_H
