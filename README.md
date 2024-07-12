# ThingHz
ThingHz IoT devices provides real time monitoirng and Automation solutions for indoor farms. It can operate over WiFi or using M2M MTE SIM. It provides secure communication with cloud using SSL device certificates. It can operateas a standalone device or can be the part of mesh network using ESP32 proprietry protocol "ESPNOW"

![thinghz_Device](https://github.com/ThingHz/ThinghzAndroid/blob/master/Thinghz_device_collage.png)
### Platform
espressif32

### Board used 
ESP32 Dev with SIM7670C

### Framework
Arduino

### Libraries used:

#### library for ds18b20
1. Dallastemperature 3.8.0 Miles Burton <https://github.com/milesburton/Arduino-Temperature-Control-Library>
2. oneWire 2.3.4 Jim Studt, Tom Pollard, Robin James, Glenn Trewitt, Jason Dangel <https://github.com/PaulStoffregen/OneWire>

#### library for Sht3x
1. Adafruit_SHT31.h 1.1.6 by adafruit <https://github.com/adafruit/Adafruit_SHT31>

#### library used for Json
1. Arduino Json 5.5.0 Benoit Blenchon <https://github.com/bblanchon/ArduinoJson>

#### library used for LTE, MQTT and SSL communication
1. vshymanskyy/TinyGSM@^0.11.5
2. vshymanskyy/StreamDebugger @ ^1.0.1
3. knolleary/PubSubClient @ ^2.8
4. knolleary/PubSubClient @ ^2.8

#### library used for BH1750 Light intensity
1. claws/BH1750@^1.3.0

#### library used for OTA
1. ayushsharma82/AsyncElegantOTA@^2.2.6

#### library used for Asynchroneous web server
1. ESPAsyncWebserver Me No Dev https://github.com/me-no-dev/ESPAsyncWebServer
2. dependency for ESP8266 ESPAsyncTCP Me No Dev https://github.com/me-no-dev/ESPAsyncTCP
3. dependency for ESP32 AsyncTCP Me No Dev https://github.com/me-no-dev/AsyncTCP

#### library used for ESPNow
1. ESP Now ESP32

### ThingHz Android Application
Know more about ThingHz IoT backend in this [github repository](https://github.com/ThingHz/ThinghzAndroid)

### ThingHz IoT Backend
Know more about ThingHz IoT backend in this [github repository](https://github.com/ThingHz/thinghzIoTBackend)

