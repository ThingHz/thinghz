#ifndef CLOUDTALKGSM_H
#define CLOUDTALKGSM_H

#include "ArduinoJson.h"
#include <SPIFFS.h>
#include "WiFiOTA.h"
#include "deviceState.h"
#include "hardwareDefs.h"
#include "utils.h"
#include "SensorPayload.h"
#include "tftState.h"

#define JSON_MSG_MAX_LEN 512
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>
#include "HTTPClient.h"

const char topic_publish[] = "aws/thing/thinghz/";
const char topic_subscribe[] = "aws/thing/thinghz/light";
const int port = 8883;

class CloudTalkGSM
{
public:
    CloudTalkGSM();
    bool setMQTTTopic(TinyGsm *modem);
    bool createMQTTPayload(TinyGsm *modem);
    bool publishToTopic(TinyGsm *modem);
    bool subscribeToTopic(TinyGsm *modem, int qos);
    bool configureSSL(TinyGsm *modem);
    bool startMQTTService(TinyGsm *modem);
    bool accquireClient(TinyGsm *modem);
    bool connectMQTT(TinyGsm *modem);
    bool releaseMQTTClient(TinyGsm *modem);
    bool openNetwork(TinyGsm *modem);
    bool stopMQTTClient(TinyGsm *modem);
    bool disconnectMQTTClient(TinyGsm *modem);
    bool updateNTPTime(TinyGsm *modem);
    String createPayload(uint8_t sProfile);
    bool restartModem(TinyGsm *modem);
    bool initialiseModem(TinyGsm *modem);
    bool handleSubscribe(char *subscribeString);
    String createSubscribeTopic(bool ack);
    void retryGPRSConnection(TinyGsm* modem);

};

#endif // CLOUDTALKGSM_H
