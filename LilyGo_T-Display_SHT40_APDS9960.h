//
// Created by Adam Howell on 2022-12-06.
//

#ifndef LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
#define LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H


void setup();
void readTelemetry();
void printTelemetry();
void readColors();
void toggleLED();
void loop();


#include <Wire.h>
//#include "C:/Code/Arduino/libraries/Adafruit_SHT4x_Library/Adafruit_SHT4x.h"
//#include <C:/Code/Arduino/libraries/Adafruit_SHT4x_Library/Adafruit_SHT4x.h>
#include "Adafruit_APDS9960.h"
#include "Adafruit_SHT4x.h"
#include <Adafruit_APDS9960.h>
#include <Adafruit_SHT4x.h>


#define OVERRIDE_WIRE  // Commend out this line to use the default SCL and SDA GPIOs.
#ifdef OVERRIDE_WIRE
const byte sdaGPIO = 43;  // Use this to set the SDA GPIO if your board uses a non-standard GPIOs for the I2C bus.
const byte sclGPIO = 44;  // Use this to set the SCL GPIO if your board uses a non-standard GPIOs for the I2C bus.
#endif


unsigned long lastTelemetryReadTime = 0;     // The last time sensors were polled.
unsigned long lastTelemetryPrintTime = 0;    // The last time sensor data was printed.
unsigned int BACKLIGHT = 38;                 // The GPIO used by the TFT backlight.
unsigned int telemetryPollInterval = 15000;  // How long to wait between sensor polling.
unsigned int telemetryPrintInterval = 5000;  // How long to wait between sensor printing.
float tempArray[] = { 0, 0, 0 };             // An array to hold the 3 most recent temperature values.
float humidityArray[] = { 0, 0, 0 };         // An array to hold the 3 most recent humidity values.


Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_APDS9960 apds;


#endif  //LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
