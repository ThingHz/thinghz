#include "test.pb.h"
#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include <AWS_IOT.h>
#include <PubSubClient.h>
#include <WiFi.h>
AWS_IOT aws_iot;
char HOST_ADDRESS[]="a26dm966t9g0lv-ats.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "thinghz1234";
char TOPIC_NAME[]= "thing";
 
void setup(){
    Serial.begin(115200);
    if(!reconnectWiFi("Sarthak","wireless18",200)){
        Serial.println("trouble connevcting to wifi");
      }
    reconnectMQTT();
    
    uint8_t buffer[128];

    
    sensorT sensor_T_message = sensorT_init_zero;


    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    sensor_T_message.sensorProfile = 1;
    sensor_T_message.has_hwRev = true;
    sensor_T_message.has_fwRev = true;
    sensor_T_message.has_deviceType = true;
    sensor_T_message.has_batteryPercentage = true;
    sensor_T_message.hwRev = 2;
    sensor_T_message.fwRev = 2;
    sensor_T_message.deviceType = 2;
    sensor_T_message.batteryPercentage = 42;
    sensor_T_message.temp = 24.3;
    
    bool status = pb_encode(&stream, sensorT_fields, &sensor_T_message);
     if (!status)
  {
      Serial.println("Failed to encode");
      return;
  }
    Serial.print("Message Length: ");
    Serial.println(stream.bytes_written);
    String protobufStr;
    for(int i = 0; i<stream.bytes_written; i++){
       char buf[2];
       sprintf(buf,"%02X",buffer[i]);
       protobufStr+=String(buf);
    }

    char messageCreatePayload[256];
    snprintf(messageCreatePayload,256,"{\"msg\":\"%s\"}",protobufStr.c_str());
    //String protoStr = protobufPayloadToString(buffer,stream.bytes_written);
    //protobufPayloadToString(buffer,stream.bytes_written);
    if(aws_iot.publish(TOPIC_NAME,messageCreatePayload) == 0)
        {        
            Serial.print("Publish Message:");
            Serial.println(messageCreatePayload);
        }
    Serial.println(protobufStr);
    
  }

  void loop(){
    }

char* protobufPayloadToString(const uint8_t buff[], int protoSize)
{
  char protobufStr[128];
  for(int i = 0; i<protoSize; i++){
       char buf[2];
       sprintf(buf,"%02X",buff[i]);
       //Serial.printf("buffer%s\n",buf);
       strcat(protobufStr,buf);
    }
    return protobufStr;
}

void reconnectMQTT(){
  
    if(aws_iot.connect(HOST_ADDRESS,CLIENT_ID)== 0)
    {
        Serial.println("Connected to AWS");
    }
delay(2000);
}

bool reconnectWiFi(const String& ssid, const String& pass, int maxDelay) {
  int loopCounter = 0;
  bool connectSuccess=true;
  WiFi.mode(WIFI_STA); 
  Serial.printf("ssid: %s", ssid.c_str());
  Serial.printf("pass: %s", pass.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("staring wait for connection");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    delay(maxDelay);
    if (loopCounter == 30) {
      Serial.println("timeout trying to connect to wifi\n");
      connectSuccess = false;
      break;
    }
    loopCounter++;
  }
  connectSuccess = WiFi.isConnected();
  if (connectSuccess) {
    Serial.println("connected to:  ");
    Serial.println(WiFi.localIP());
  }
  return connectSuccess;
}
