#include "sensorPayload.h"

char *createSensorTpaylaod(T_SensorPayload *t_sensorPayloadT, char *messagePayload, size_t SIZE)
{
    t_sensorPayloadT->t_sensorTH->sensorPayloadT->temp = RSTATE.temperature;
    t_sensorPayloadT->sensorProfile = SensorTemp;
    t_sensorPayloadT->hwRev = Thinghz_Plant_IQ;
    t_sensorPayloadT->fwRev = Thinghz_Farmsense_PlantIQ_T;
    snprintf(messagePayload,
             SIZE,
             "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
             macAddrWithoutColons().c_str(),
             t_sensorPayloadT->t_sensorTH->sensorPayloadT->temp,
             t_sensorPayloadT->sensorProfile,
             RSTATE.batteryPercentage);
    return messagePayload;
}

char *createSensorTHpayload(T_SensorPayload *t_sensorPayloadTH, char *messagePayload, size_t SIZE)
{
    t_sensorPayloadTH->t_sensorTH->humidity = RSTATE.humidity;
    t_sensorPayloadTH->t_sensorTH->sensorPayloadT->temp = RSTATE.temperature;
    t_sensorPayloadTH->sensorProfile = SensorTH;
    t_sensorPayloadTH->hwRev = Thinghz_Plant_IQ;
    t_sensorPayloadTH->fwRev = Thinghz_Farmsense_PlantIQ_TH;
    snprintf(messagePayload,
             SIZE,
             "{\"device_id\":\"%s\",\"temp\": \"%.1f\",\"humid\": \"%.1f\",\"sensor_profile\": %d,\"battery\": \"%d\"}",
             macAddrWithoutColons().c_str(),
             t_sensorPayloadTH->t_sensorTH->sensorPayloadT->temp,
             t_sensorPayloadTH->t_sensorTH->humidity,
             t_sensorPayloadTH->sensorProfile,
             RSTATE.batteryPercentage);
    return messagePayload;
}

char *createSensorGasPayload(T_SensorPayload *t_sensorPayloadGas, char *messagePayload, size_t SIZE)
{
    t_sensorPayloadGas->t_sensorGas->gas = RSTATE.carbon;
    t_sensorPayloadGas->t_sensorTH->humidity = RSTATE.humidity;
    t_sensorPayloadGas->t_sensorTH->sensorPayloadT->temp = RSTATE.temperature;
    t_sensorPayloadGas->sensorProfile = SensorGas;
    t_sensorPayloadGas->hwRev = Thinghz_Plant_IQ;
    t_sensorPayloadGas->fwRev = Thinghz_Farmsense_PlantIQ_TH;
    snprintf(messagePayload,
             SIZE,
             "{\"device_id\":\"%s\",\"gas\":\"%u\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d}",
             macAddrWithoutColons().c_str(),
             t_sensorPayloadGas->t_sensorGas->gas,
             t_sensorPayloadGas->t_sensorTH->sensorPayloadT->temp,
             t_sensorPayloadGas->t_sensorTH->humidity,
             RSTATE.batteryPercentage,
             t_sensorPayloadGas->sensorProfile);
    return messagePayload;
}

char *createSensorLightPayload(T_SensorPayload *t_sensorPayloadLight, char *messagePayload, size_t SIZE)
{
    t_sensorPayloadLight->t_sensorLight->lux = RSTATE.lux;
    t_sensorPayloadLight->t_sensorLight->lightState1 = !RSTATE.light_state_1;
    t_sensorPayloadLight->t_sensorLight->lightState2 = !RSTATE.light_state_2;
    t_sensorPayloadLight->t_sensorLight->lightState3 = !RSTATE.light_state_3;
    t_sensorPayloadLight->t_sensorLight->lightState4 = !RSTATE.light_state_4;
    t_sensorPayloadLight->t_sensorTH->humidity = RSTATE.humidity;
    t_sensorPayloadLight->t_sensorTH->sensorPayloadT->temp = RSTATE.temp;
    t_sensorPayloadLight->hwRev = Thinghz_Plant_IQ;
    t_sensorPayloadLight->fwRev = Thinghz_Farmsense_PlantIQ_THL;
    snprintf(messagePayload, 
                SIZE, 
                "{\"device_id\":\"%s\",\"lux\":\"%.1f\",\"temp\":\"%.1f\",\"humid\":\"%.1f\",\"battery\":\"%d\",\"sensor_profile\":%d,\"light_state_1\":%u,\"light_state_2\":%u,\"light_state_3\":%u,\"light_state_4\":%u}" ),
                macAddrWithoutColons().c_str(),
                t_sensorPayloadLight->t_sensorLight->lux,
                t_sensorPayloadLight->t_sensorTH->sensorPayloadT->temp,
                t_sensorPayloadLight->t_sensorTH->humidity,
                RSTATE.batteryPercentage,
                t_sensorPayloadLight->t_sensorLight->lightState1,
                t_sensorPayloadLight->t_sensorLight->lightState2,
                t_sensorPayloadLight->t_sensorLight->lightState3,
                t_sensorPayloadLight->t_sensorLight->lightState4
    );

    return messagePayload;
}