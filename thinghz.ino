#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#define JSON_MSG_MAX_LEN 256
#define FIN_SENSOR_PIN 32
#include <OneWire.h>
#include <DallasTemperature.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();
float temp = 0.0;

int counter =0;
bool isSHTThere = false;
const char *ssid = "Agnextsn_compass";
const char *pass = "Agnext0419";
const char urlmessageSend[] = "https://api.thinghz.com/v1/data"; 
char messagePayload[JSON_MSG_MAX_LEN];
const char COOLNEXT_MESSAGE_BODY[] = "{\"Item\":{\"temp\": \"%.1f\",\"humid\": \"%.1f\",\"profile\": %d,\"battery\": \"%d\"}}";
OneWire           tempWire(FIN_SENSOR_PIN);
DallasTemperature tempSensor(&tempWire);



void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial.println("Started");
Serial.println("SHT31 test");
tempSensor.begin();
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    isSHTThere = false;
    }else{
        isSHTThere = true;
      }
if(reconnectWiFi()){
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(isSHTThere){
      float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.println(t);
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }
  Serial.println();
  
    }
  tempSensor.requestTemperatures();
  temp = tempSensor.getTempCByIndex(0);
  Serial.println(temp);
  if (isnan(temp)) {
    Serial.println("DSB fail");
  }
  if(!sendData()){
    Serial.println("No http connection");
  }
  delay(5000);
}

bool reconnectWiFi(){
  int loopCounter = 0;
  delay(500);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);  // fixem:: why AP STA mandeep had it like this.
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    delay(500);
    if (loopCounter == 30) {
      Serial.println("timeout trying to connect to wifi");
      return false;
      break;
    }
    loopCounter++;
  }
  return true;
}

bool sendData(){
  snprintf(messagePayload,JSON_MSG_MAX_LEN, COOLNEXT_MESSAGE_BODY, 
               temp,
               98.4,
               0,
               92
               );
      HTTPClient http;
      http.begin(urlmessageSend);//zx     //test
      http.addHeader("Content-Type" , "application/json");  //Specify content-type header
      Serial.println(messagePayload);
      int httpCode = http.POST(messagePayload);     //Send the request
      if (httpCode == HTTP_CODE_OK && httpCode > 0) {
        Serial.println( http.getString());   //Get the response payload
      } else {
        Serial.printf("[HTTP] POST... failed, error: %s and code is %d\n", http.errorToString(httpCode).c_str(), httpCode);
        return false;
      }
      http.end();
  }  
