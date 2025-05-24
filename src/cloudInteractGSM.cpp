#include "cloudInteractGSM.h"

// Function implementations


bool CloudTalkGSM::setMQTTTopic(TinyGsm *modem)
{
    modem->sendAT(GF("+CMQTTTOPIC=0,21"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
        return false;
    size_t topic_size = strlen(topic_publish);
    modem->stream.write(topic_publish, topic_size);                                            
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
}

bool CloudTalkGSM::createMQTTPayload(TinyGsm *modem)
{
    String payload = createPayload(DEVICE_SENSOR_TYPE);
    size_t payload_size = strlen(payload.c_str());
    modem->sendAT(GF("+CMQTTPAYLOAD=0"), ',', payload_size);
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
        return false;
    modem->stream.write(payload.c_str(), payload_size);
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    return true;
}

bool CloudTalkGSM::publishToTopic(TinyGsm *modem)
{
    DEBUG_PRINTLN("MQTT publishing message");
    modem->sendAT(GF("+CMQTTPUB=0,1,60"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTPUB: 0,0"));
    if (!res)
    {
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
        return false;
    }
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_MessagePublishFailed);
    return res;
}

bool CloudTalkGSM::subscribeToTopic(TinyGsm *modem, int qos)
{
    size_t topic_size = strlen(topic_subscribe);
    modem->sendAT(GF("+CMQTTSUB=0"), ',', topic_size, ',', qos);
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, GF(">")) != 1)
        return false;
    modem->stream.write(topic_subscribe, topic_size);
    modem->stream.write(GSM_NL);
    modem->stream.flush();
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTSUB: 0,0"));
    if (!res)
    {
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SubscribeFailed);
        return false;
    }
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SubscribeFailed);
    return res;
}

bool CloudTalkGSM::configureSSL(TinyGsm *modem)
{
    DEBUG_PRINTLN("Configure SSL version");
    modem->sendAT(GF("+CSSLCFG=\"sslversion\",0,4"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;

    DEBUG_PRINTLN("Configure Auth mode");
    modem->sendAT(GF("+CSSLCFG=\"authmode\",0,2"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;

    DEBUG_PRINTLN("Configure SSL cacert");
    modem->sendAT(GF("+CSSLCFG=\"cacert\",0,\"" CACERT_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;

    Serial.println("Configure SSL clientcert");
    modem->sendAT(GF("+CSSLCFG=\"clientcert\",0,\"" CLIENTCERT_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;

    Serial.println("Configure SSL clientkey");
    modem->sendAT(GF("+CSSLCFG=\"clientkey\",0,\"" CLIENTKEY_FILE_NAME "\""));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;

    return true;
}

bool CloudTalkGSM::startMQTTService(TinyGsm *modem)
{
    DEBUG_PRINTLN("Starting MQTT");
    modem->sendAT(GF("+CMQTTSTART"));
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTSTART:"));
    if (!res)
    {
        DEBUG_PRINTLN("Could not find +CMQTTSTART");
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    }
    Serial.println("Set SSL Configuration");
    modem->sendAT(GF("+CMQTTSSLCFG=0,0"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    return true;
}

bool CloudTalkGSM::accquireClient(TinyGsm *modem)
{
    modem->sendAT(GF("+CMQTTACCQ=0,\"" MQTT_CLIENT_NAME "\",1"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1)
        return false;
    return true;
}

bool CloudTalkGSM::connectMQTT(TinyGsm *modem)
{
    modem->sendAT(GF("+CMQTTCONNECT=0,\"" MQTT_HOST "\",90,1"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTCONNECT: 0,0"));
    if (!res)
    {
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
    }
    else
    {
        clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
    }
    return res;
}

bool CloudTalkGSM::releaseMQTTClient(TinyGsm *modem)
{
    DEBUG_PRINTLN("Release Mqtt Client");
    modem->sendAT(GF("+CMQTTREL?"));
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
}

bool CloudTalkGSM::openNetwork(TinyGsm *modem)
{
    modem->sendAT(GF("+NETOPEN"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS, ("+NETOPEN: 0")) != 1)
        return false;
    return true;
}

bool CloudTalkGSM::stopMQTTClient(TinyGsm *modem)
{
    DEBUG_PRINTLN("Stop Mqtt Client");
    modem->sendAT(GF("+CMQTTSTOP"));
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
}

bool CloudTalkGSM::disconnectMQTTClient(TinyGsm *modem)
{
    DEBUG_PRINTLN("Stop Mqtt Client");
    modem->sendAT(GF("+CMQTTDISC=0,120"));
    modem->waitResponse();
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS, GF(GSM_NL "+CMQTTDISC: 0,0"));
    return res;
}

bool CloudTalkGSM::updateNTPTime(TinyGsm *modem)
{
    DEBUG_PRINTLN("Update NTP Time");
    modem->sendAT("+CNTP=\"" NTP_SERVER "\",32");
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
}

String CloudTalkGSM::createPayload(uint8_t sProfile)
{
    char* messageCreatePayload = nullptr;
    T_SensorPayload* sensorPayload = nullptr;
    messageCreatePayload = (*fp_CreatePayload[sProfile])(&sensorPayload, messageCreatePayload, JSON_MSG_MAX_LEN);
    return String(messageCreatePayload);
}

bool CloudTalkGSM::restartModem(TinyGsm *modem)
{
    int ret = modem->restart();
    String modemInfo = modem->getModemInfo();
    DEBUG_PRINTF("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
    if (modem->getSimStatus() == 0)
    {
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    else
    {
        clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    return ret;
}

void CloudTalkGSM::retryGPRSConnection(TinyGsm *modem){
     if(!modem->isGprsConnected()){
      DEBUG_PRINTLN(F("GPRS not connected"));
      modem->gprsConnect("airteliot.com");
  }
}

bool CloudTalkGSM::initialiseModem(TinyGsm *modem)
{
    DEBUG_PRINTLN("initialising Modem");
    int ret = modem->init();
    String modemInfo = modem->getModemInfo();
    DEBUG_PRINTF("modemInfo: %s\n SimStatus: %d\n", modemInfo.c_str(), modem->getSimStatus());
    if (modem->getSimStatus() == 0)
    {
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    else
    {
        clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_SimStatusZero);
    }
    return ret;
}

bool CloudTalkGSM::handleSubscribe(char *subscribeString)
{
    RSTATE.displayEvents = DisplayEventActionReceived;
    String clean_subscribeString = String(subscribeString).substring(0, (String(subscribeString).indexOf("}")) + 1);
    DEBUG_PRINTLN(clean_subscribeString.c_str());
    drawDisplay(RSTATE.displayEvents);
    StaticJsonDocument<1024> subscribeJson;
    DeserializationError err = deserializeJson(subscribeJson, subscribeString);

    if (err)
    {
        DEBUG_PRINTF("deserializeJson() failed : %s\n", err.c_str());
        return false;
    }
    else
    {
        DEBUG_PRINTLN("deserializeJson() success");
    }

    DEBUG_PRINTLN("Obtaining action variables");
    int lightState1 = 0;
    int lightState2 = 0;
    int lightState3 = 0;
    int lightState4 = 0;
    if (subscribeJson.containsKey("light_state_1"))
    {
        lightState1 = subscribeJson["light_state_1"];
    }
    if (subscribeJson.containsKey("light_state_2"))
    {
        lightState2 = subscribeJson["light_state_2"];
    }
    if (subscribeJson.containsKey("light_state_3"))
    {
        lightState3 = subscribeJson["light_state_3"];
    }
    if (subscribeJson.containsKey("light_state_4"))
    {
        lightState4 = subscribeJson["light_state_4"];
    }
    toggleLightState((uint8_t)lightState1, (uint8_t)lightState2, (uint8_t)lightState3, (uint8_t)lightState4);

    return true;
}

String CloudTalkGSM::createSubscribeTopic(bool ack)
{
    char subscribeTopic[128];
    char ackTopic[128];
    if (ack)
    {
        snprintf(ackTopic, 128, "aws/thing/thinghz/%s/ack", macAddrWithoutColons().c_str());
        return String(ackTopic);
    }
    snprintf(subscribeTopic, 128, "aws/thing/thinghz/%s/light", macAddrWithoutColons().c_str());
    return String(subscribeTopic);
}
