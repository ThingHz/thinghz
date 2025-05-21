#ifndef OLED_STATE_H_
#define OLED_STATE_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "utils.h"
#include "hardwareDefs.h"
#include "deviceState.h"
#include "WiFiOTA.h"

class OLEDDisplay {
public:
    OLEDDisplay();
    bool initDisplay();
    bool isDisplayAvailable();
    void clearDisplay();
    void drawThermometer(float Value);
    void drawDroplet(float value);
    void drawLine(uint16_t cursorPositionY, uint16_t cursorPositionX, int16_t width, int16_t height);
    void drawCross(uint16_t cursorPositionX, uint16_t cursorPositionY);
    void drawAnotation(String annotation, int16_t cursorPositionY, int16_t cursorPositionX);
    void drawSignal(uint16_t cursorPositionY, uint16_t cursorPositionX, int numLines);
    void drawLoGo(String annotation);
    void drawTag(String annotation);
    void drawValue(String value, uint16_t cursorPositionY, uint16_t cursorPositionX, uint8_t textSize);
    void drawThinghzBitmap(const unsigned char* bitmapArray, uint16_t cursorPositionY, uint16_t cursorPositionX);
    void drawDisplay(DisplayMode mode);

private:
    Adafruit_SSD1306 display;
    const unsigned char thinghz_bitmap[] PROGMEM;
};

#endif // OLED_STATE_H_
