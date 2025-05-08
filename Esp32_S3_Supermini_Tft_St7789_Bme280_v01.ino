/*
  This sketch shows how to work with the ESP32-S3 Supermini development board.
  Attached is an 2.0 inch TFT display that runs a ST7789 driver.
  The display has a size of 240x320 pixels.
  The library for the display is TFT_eSPI in a modified version, because the original
  version (2.5.4) can't run on ESP32 SDK 3.x so far and this SDK version is needed to
  select ESP32-C6 boards.

  Purpose of the sketch:
  - read the temperature, humidity and barometric air pressure from an attached BME280 sensor
  - display the temperature and humdity on the TFT display with an Analog Meter
  - display the air pressure on the TFT display with a Linear Analog Meter

  This is tested with ESP32 Board version 3.2.0 with SDK 5.4.1 on Arduino IDE 2.3.6
  TFT_eSPI version: 2.5.43
  Adafruit_BME280_Library version:  2.2.45
*/

/*
Version Management
08.05.2025 V01 Initial programming
*/

/*
TFT 240 x 320 pixels 2.0 inch ST7789 display wiring to an ESP32-S3 Supermini
Terminals on display's pcb from left to right

TFT   ESP32-S3
GND   GND
VDD   3.3V 
SCL   13
SDA   12 (= "MOSI")
RST   11
DC    10
CS    9
BLK   8 *1)

Note *1) If you don't need a dimming you can connect BLK with 3.3V
Note *2) The display does not have a MISO ("output") terminal, so it is not wired
*/

// -------------------------------------------------------------------------------
// Sketch and Board information
const char *PROGRAM_VERSION = "ESP32-S3 Supermini ST7789 BME280 V01";
const char *PROGRAM_VERSION_SHORT = "ESP32-S3 ST7789 BME280 V01";
const char *DIVIDER = "--------------------";

// -------------------------------------------------------------------------------
// TFT Display

#include <TFT_eSPI.h>     // Hardware-specific library
#include <TFT_eWidget.h>  // Widget library https://github.com/Bodmer/TFT_eWidget/tree/main
#include <SPI.h>

#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define TFT_GREY 0x2104  // Dark grey 16 bit colour

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library with default width and height

#define TFT_BL_PIN 8               // backlight brightness control, needs to be a PWM pin
#define TFT_BRIGHTNESS_PERCENT 90  // avoids overheating of the device

#define TFT_PRESSURE_METER_START 266

MeterWidget temp = MeterWidget(&tft);
MeterWidget hum = MeterWidget(&tft);
float presMap;

#define LOOP_PERIOD 35  // Display updates every 35 ms

// -------------------------------------------------------------------------------
// BME280 sensor

#define BME280_I2C_SDA_PIN 1
#define BME280_I2C_SCL_PIN 2
#define BME280_I2C_ADDRESS 0x76

#include <Wire.h>
#include <Adafruit_Sensor.h>  // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h>  // https://github.com/adafruit/Adafruit_BME280_Library
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;  // I2C

float temperature = -99;
float humidity = -99;
float pressure = -99;
float altitude = -99;

int interval = 2000;

void getBme280Values() {
  bme.takeForcedMeasurement();
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void printBme280Values() {
  Serial.print(F("Temperature: "));
  Serial.print(temperature, 1);
  Serial.print(F("c, Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%, Pressure: "));
  Serial.print(pressure, 0);
  Serial.print(F("hPa, Altitude: "));
  Serial.print(altitude, 1);
  Serial.println(F("m"));
  Serial.flush();
}

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  Serial.println(PROGRAM_VERSION);

  tft.begin();

  // set the brightness to 90% to avoid heating of the device
  pinMode(TFT_BL_PIN, OUTPUT);
  analogWrite(TFT_BL_PIN, 255 * TFT_BRIGHTNESS_PERCENT / 100);
  delay(10);

  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawCentreString(PROGRAM_VERSION_SHORT, 120, 0, 2);

  temp.analogMeter(0, 14, 30.0, "Temp °C", "0", "7.5", "15.0", "22.5", "30.0");   // Draw analogue meter at 0, 0
  hum.analogMeter(0, 136, 100.0, "Hum %", "0", "25.0", "50.0", "75.0", "100.0");  // Draw analogue meter at 0, 0

  displayPressureLinearMeter();

  bool wireStatus = Wire.begin(BME280_I2C_SDA_PIN, BME280_I2C_SCL_PIN);
  if (!wireStatus) {
    Serial.println(F("Wire begin failed"));
    while (1)
      ;
  } else {
    Serial.println(F("Wire begin running"));
  }

  delay(1000);

  bool bme_status = bme.begin(BME280_I2C_ADDRESS);  // address either 0x76 or 0x77
  if (!bme_status) {

    Serial.println(F("No valid BME280 found"));
    while (1)
      ;
  } else {
    Serial.println(F("BME280 found"));
  }
  // the preferred way to get correct indoor temperatures
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X16,  // temperature
                  Adafruit_BME280::SAMPLING_X1,   // pressure
                  Adafruit_BME280::SAMPLING_X1,   // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_0_5);
}

void loop() {

  if ((millis() - interval) > 1000) {

    getBme280Values();
    printBme280Values();

    temp.updateNeedle(temperature, 0);
    hum.updateNeedle(humidity, 0);
    displayPressureLinearMeter();

    interval = millis();
  }
}

float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax) {
  return tomin + (((tomax - tomin) * (ip - ipmin)) / (ipmax - ipmin));
}

void displayPressureLinearMeter() {
  String presString = " Air Pressure " + String (pressure, 0) + " hPa ";
    tft.drawCentreString(presString, 120, TFT_PRESSURE_METER_START, 2);
    presMap = mapValue(pressure, (float)900.0, (float)1100.0, (float)0.0, (float)30.0);
    // air pressure
    linearMeter(presMap, 1, TFT_PRESSURE_METER_START + 18, 5, 20, 3, 30, GREEN2GREEN);
    // draw a line for a scale
    tft.drawLine(4, TFT_PRESSURE_METER_START + 42, 256, TFT_PRESSURE_METER_START + 42, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(" 900      950     1000     1050    1100", 4, TFT_PRESSURE_METER_START + 44);
}

// Linear Analoge Meters

// #########################################################################
//  Draw the linear meter
// #########################################################################
// val =  reading to show (range is 0 to n)
// x, y = position of top left corner
// w, h = width and height of a single bar
// g    = pixel gap to next bar (can be 0)
// n    = number of segments
// s    = colour scheme
void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s) {
  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;
  // Draw n colour blocks
  for (int b = 1; b <= n; b++) {
    if (val > 0 && b <= val) {  // Fill in coloured blocks
      switch (s) {
        case 0: colour = TFT_RED; break;                             // Fixed colour
        case 1: colour = TFT_GREEN; break;                           // Fixed colour
        case 2: colour = TFT_BLUE; break;                            // Fixed colour
        case 3: colour = rainbowColor(map(b, 0, n, 127, 0)); break;  // Blue to red
        case 4: colour = rainbowColor(map(b, 0, n, 63, 0)); break;   // Green to red
        case 5: colour = rainbowColor(map(b, 0, n, 0, 63)); break;   // Red to green
        case 6: colour = rainbowColor(map(b, 0, n, 0, 159)); break;  // Rainbow (red to violet)
      }
      tft.fillRect(x + b * (w + g), y, w, h, colour);
    } else  // Fill in blank segments
    {
      tft.fillRect(x + b * (w + g), y, w, h, TFT_DARKGREY);
    }
  }
}

/***************************************************************************************
** Function name:           rainbowColor
** Description:             Return a 16 bit rainbow colour
***************************************************************************************/
// If 'spectrum' is in the range 0-159 it is converted to a spectrum colour
// from 0 = red through to 127 = blue to 159 = violet
// Extending the range to 0-191 adds a further violet to red band

uint16_t rainbowColor(uint8_t spectrum) {
  spectrum = spectrum % 192;

  uint8_t red = 0;    // Red is the top 5 bits of a 16 bit colour spectrum
  uint8_t green = 0;  // Green is the middle 6 bits, but only top 5 bits used here
  uint8_t blue = 0;   // Blue is the bottom 5 bits

  uint8_t sector = spectrum >> 5;
  uint8_t amplit = spectrum & 0x1F;

  switch (sector) {
    case 0:
      red = 0x1F;
      green = amplit;  // Green ramps up
      blue = 0;
      break;
    case 1:
      red = 0x1F - amplit;  // Red ramps down
      green = 0x1F;
      blue = 0;
      break;
    case 2:
      red = 0;
      green = 0x1F;
      blue = amplit;  // Blue ramps up
      break;
    case 3:
      red = 0;
      green = 0x1F - amplit;  // Green ramps down
      blue = 0x1F;
      break;
    case 4:
      red = amplit;  // Red ramps up
      green = 0;
      blue = 0x1F;
      break;
    case 5:
      red = 0x1F;
      green = 0;
      blue = 0x1F - amplit;  // Blue ramps down
      break;
  }

  return red << 11 | green << 6 | blue;
}
