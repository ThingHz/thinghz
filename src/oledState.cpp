#include "oledState.h"

// Bitmap data
const unsigned char OLEDDisplay::thinghz_bitmap[] PROGMEM = {
    // Bitmap data here...
};

OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool OLEDDisplay::initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DEBUG_PRINTLN("SSD1306 allocation failed");
        setBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
        return false;
    }
    clearBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
    return true;
}

bool OLEDDisplay::isDisplayAvailable() {
    return !testBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
}

void OLEDDisplay::clearDisplay() {
    delay(100); //TODO:: don't know why this is needed.
    display.clearDisplay();
    display.setTextColor(WHITE);
}

void OLEDDisplay::drawThermometer(float Value) {
    display.fillCircle(8, 55, 3, WHITE);
    display.drawRoundRect(6, 3, 5, 49, 2, WHITE);
    Value = Value * 0.43; // ratio for show
    display.drawLine(8, 46, 8, 46 - Value, WHITE);
    display.setFont();
    display.setCursor(110, 20);
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");
}

void OLEDDisplay::drawDroplet(float value) {
    display.fillCircle(8, 55, 7, WHITE);
    display.fillTriangle(8, 48, 4, 55, 12, 55, WHITE);
}

void OLEDDisplay::drawLine(uint16_t cursorPositionY, uint16_t cursorPositionX, int16_t width, int16_t height) {
    display.fillRect(cursorPositionX, cursorPositionY, width, height, WHITE);
}

void OLEDDisplay::drawCross(uint16_t cursorPositionX, uint16_t cursorPositionY) {
    display.drawLine(cursorPositionX + 3, cursorPositionY - 3, cursorPositionX, cursorPositionY + 3, WHITE);
    display.drawLine(cursorPositionX, cursorPositionY - 3, cursorPositionX + 3, cursorPositionY + 3, WHITE);
}

void OLEDDisplay::drawAnotation(String annotation, int16_t cursorPositionY, int16_t cursorPositionX) {
    display.setFont();
    display.setTextSize(1);
    display.setCursor(cursorPositionX, cursorPositionY);
    display.println(annotation);
}

void OLEDDisplay::drawSignal(uint16_t cursorPositionY, uint16_t cursorPositionX, int numLines) {
    switch (numLines) {
        case 0:
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 10, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 6, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 3, 1, WHITE);
            display.drawPixel(cursorPositionX + 27, cursorPositionY - 10, WHITE);
            drawCross(cursorPositionX + 16, cursorPositionY - 5);
            display.fillRect(cursorPositionX + 13, cursorPositionY + 30, 12, 7, WHITE);
            display.fillRect(cursorPositionX + 11, cursorPositionY + 32, 4, 3, WHITE);
            break;
        case 1:
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 10, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 6, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 3, 1, WHITE);
            display.drawPixel(cursorPositionX + 27, cursorPositionY - 10, WHITE);
            display.fillRect(cursorPositionX + 13, cursorPositionY + 30, 12, 7, WHITE);
            display.fillRect(cursorPositionX + 11, cursorPositionY + 32, 4, 3, WHITE);
            break;
        case 2:
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 10, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 6, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 3, 1, WHITE);
            display.drawPixel(cursorPositionX + 27, cursorPositionY - 10, WHITE);
            drawAnotation("4G", cursorPositionY - 7, cursorPositionX + 15);
            drawAnotation("1", cursorPositionY + 4, cursorPositionX + 13);
            if (!RSTATE.light_state_1) {
                display.fillCircle(cursorPositionX + 22, cursorPositionY + 6, 2, WHITE);
            }
            drawAnotation("2", cursorPositionY + 14, cursorPositionX + 13);
            if (!RSTATE.light_state_2) {
                display.fillCircle(cursorPositionX + 22, cursorPositionY + 16, 2, WHITE);
            }
            display.fillRect(cursorPositionX + 13, cursorPositionY + 30, 12, 7, WHITE);
            display.fillRect(cursorPositionX + 11, cursorPositionY + 32, 4, 3, WHITE);
            break;
        default:
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 10, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 6, 1, WHITE);
            display.drawCircleHelper(cursorPositionX + 27, cursorPositionY - 10, 3, 1, WHITE);
            display.drawPixel(cursorPositionX + 27, cursorPositionY - 10, WHITE);
            display.fillRect(cursorPositionX + 13, cursorPositionY + 30, 12, 7, WHITE);
            display.fillRect(cursorPositionX + 11, cursorPositionY + 32, 4, 3, WHITE);
            break;
    }
}

void OLEDDisplay::drawLoGo(String annotation) {
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setCursor(20, 20);
    display.print(annotation);
}

void OLEDDisplay::drawTag(String annotation) {
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(25, 58);
    display.print(annotation);
}

void OLEDDisplay::drawValue(String value, uint16_t cursorPositionY, uint16_t cursorPositionX, uint8_t textSize) {
    display.setFont(&FreeSerif9pt7b);
    display.setTextSize(textSize);
    display.setTextColor(WHITE);
    display.setCursor(cursorPositionX, cursorPositionY);
    display.print(value);
}

void OLEDDisplay::drawThinghzBitmap(const unsigned char* bitmapArray, uint16_t cursorPositionY, uint16_t cursorPositionX) {
    display.drawBitmap(cursorPositionX, cursorPositionY, bitmapArray, 125, 35, BLACK, WHITE);
}

void OLEDDisplay::drawDisplay(DisplayMode mode) {
    display.clearDisplay();
    char valStr[20] = {0};
    switch (mode) {
        case DisplayTempHumid: {
            drawAnotation("Temperature", 0, 0);
            snprintf(valStr, 20, "%.1f C", RSTATE.temperature);
            drawValue(valStr, 25, 0, 1);
            drawLine(30, 0, 128, 3);
            drawAnotation("Humidity", 34, 0);
            drawAnotation("Battery", 34, 75);
            snprintf(valStr, 20, "%.1f%c", RSTATE.humidity, 37);
            drawValue(valStr, 58, 0, 1);
            snprintf(valStr, 20, "%d%c", RSTATE.batteryPercentage, 37);
            drawValue(valStr, 58, 75, 1);
        } break;
        case DisplayTempHumiCO2: {
            drawAnotation("Temp", 0, 0);
            snprintf(valStr, 20, "%.1fC", RSTATE.temperature);
            drawValue(valStr, 25, 0, 1);
            drawAnotation("Humid", 0, 65);
            snprintf(valStr, 20, "%.1f%c", RSTATE.humidity, 37);
            drawValue(valStr, 25, 65, 1);
            drawLine(30, 0, 128, 3);
            drawAnotation("CO2", 34, 0);
            snprintf(valStr, 20, "%dppm", RSTATE.carbon);
            drawValue(valStr, 58, 0, 1);
        } break;
        case DisplayTempHumiLux: {
            drawAnotation("Temp", 0, 0);
            snprintf(valStr, 20, "%.1fC", RSTATE.temperature);
            drawValue(valStr, 25, 0, 1);
            drawAnotation("Humidity", 0, 52);
            snprintf(valStr, 20, "%.1f%c", RSTATE.humidity, 37);
            drawValue(valStr, 25, 52, 1);
            if (!RSTATE.isNetworkActive) {
                DEBUG_PRINTF("signal value is %d\n", 0);
                drawSignal(20, 100, 0);
            } else if (RSTATE.isMQTTConnected && RSTATE.isNetworkActive) {
                DEBUG_PRINTF("signal value is %d\n", 2);
                drawSignal(20, 100, 2);
            } else {
                DEBUG_PRINTF("signal value is %d\n", 1);
                drawSignal(20, 100, 1);
            }
            drawLine(0, 107, 1, 64);
            drawAnotation("Lux", 34, 0);
            snprintf(valStr, 20, "%.1fLx", RSTATE.lux);
            drawValue(valStr, 58, 0, 1);
            drawAnotation("Light", 34, 62);
            if (!RSTATE.light_state_1) {
                drawValue("on", 58, 68, 1);
            } else {
                drawValue("off", 58, 68, 1);
            }
        } break;
        case DisplayTemp: {
            drawAnotation("Temperature", 0, 0);
            snprintf(valStr, 20, "%.1fC", RSTATE.temperature);
            drawValue(valStr, 30, 20, 1);
            drawLine(30, 0, 128, 3);
            drawAnotation("Bat", 34, 0);
            snprintf(valStr, 20, "%d%c", RSTATE.batteryPercentage, 37);
            drawValue(valStr, 58, 0, 1);
        } break;
        case DisplayGas: {
            drawAnotation("Gas", 0, 0);
            snprintf(valStr, 20, "%.1f", 34.5);
            drawValue(valStr, 30, 20, 1);
        } break;
        case DisplayCenterTextLogo: {
            drawThinghzBitmap(thinghz_bitmap, 0, 0);
            drawAnotation("ForRemoteSupervision", 35, 5);
            drawTag("Mushbot");
        } break;
        case DisplayDeviceHealth: {
            drawAnotation("Making Network Connection...", 5, 30);
        } break;
        case DisplayDeviceStatus: {
            drawAnotation("Connected", 5, 10);
            delay(4000);
        } break;
        case DisplayPortalConfig: {
            drawAnotation("Portal Active", 10, 5);
            drawAnotation("192.168.4.1", 30, 5);
            drawAnotation(macAddrWithoutColons(), 50, 5);
            delay(4000);
        } break;
        case DisplayEventActionReceived: {
            drawAnotation("Light", 10, 5);
            drawAnotation("Action Received", 30, 5);
        } break;
        default: {
            display.setTextSize(1);
            display.setCursor(20, 0);
            display.print("NO Mode");
        } break;
    }
    yield();
    delay(100);
    display.display();
}
