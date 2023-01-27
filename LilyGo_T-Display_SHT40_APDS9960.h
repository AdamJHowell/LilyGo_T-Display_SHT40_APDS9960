//
// Created by Adam Howell on 2022-12-06.
//

#ifndef LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
#define LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H

#include "Adafruit_APDS9960.h"
#include "Adafruit_SHT4x.h"
#include "ESPmDNS.h"
#include "privateInfo.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>


/**
 * Function declarations.
 */
// Functions in NetworkFunctions.ino
void mqttCallback( char *topic, byte *payload, unsigned int length );
void onReceiveCallback( char *topic, byte *payload, unsigned int length );
void configureOTA();
int checkForSSID( const char *ssidName );
void wifiConnect();
void mqttConnect();
void publishStats();
void publishTelemetry();
void lookupWifiCode( int code, char *buffer );
void lookupMQTTCode( int code, char *buffer );

// Functions in LillyGo_T-Display_SHT40_APDS9960.ino
void setup();
void setupAPDS();
void setupSht40();
void readTelemetry();
void printTelemetry();
void readColors();
void toggleLED();
void loop();


#define OVERRIDE_WIRE // Commend out this line to use the default SCL and SDA GPIOs.
#ifdef OVERRIDE_WIRE
const byte sdaGPIO = 43; // Use this to set the SDA GPIO if your board uses a non-standard GPIOs for the I2C bus.
const byte sclGPIO = 44; // Use this to set the SCL GPIO if your board uses a non-standard GPIOs for the I2C bus.
#endif

const unsigned int BACKLIGHT = 38;									 // The GPIO used by the TFT backlight.
const unsigned int wifiConnectionTimeout = 10000;				 // The maximum amount of time to wait for a Wi-Fi connection before considering the attempt to have failed.
const unsigned int JSON_DOC_SIZE = 512;							 // The ArduinoJson document size, and size of some buffers.
const char *HOST_NAME = "T-Display-S3";							 // The hostname used for networking.
const char *MQTT_STATS_TOPIC = "T-Display-S3/stats";			 // The topic this device will publish to upon connection to the broker.
const char *MQTT_COMMAND_TOPIC = "T-Display-S3/commands";	 // The command topic this device will subscribe to.
const char *commandTopic = "T-Display-S3/commands";			 // The topic used to subscribe to update commands.  Commands: publishTelemetry, changeTelemetryInterval, publishStatus.
const char *sketchTopic = "T-Display-S3/sketch";				 // The topic used to publish the sketch name (__FILE__).
const char *macTopic = "T-Display-S3/mac";						 // The topic used to publish the MAC address.
const char *ipTopic = "T-Display-S3/ip";							 // The topic used to publish the IP address.
const char *rssiTopic = "T-Display-S3/rssi";						 // The topic used to publish the Wi-Fi Received Signal Strength Indicator.
const char *publishCountTopic = "T-Display-S3/publishCount"; // The topic used to publish the loop count.
const char *notesTopic = "T-Display-S3/notes";					 // The topic used to publish notes relevant to this project.
unsigned long wifiConnectCount = 0;									 // A counter for how many times the wifiConnect() function has been called.
unsigned long mqttConnectCount = 0;									 // A counter for how many times the mqttConnect() function has been called.
unsigned long printCount = 0;											 // A counter of how many times the stats have been printed.
unsigned long publishCount = 0;										 // A counter of how many times the stats have been printed.
unsigned long callbackCount = 0;										 // The number of times a callback was received.
unsigned long lastWifiConnectTime = 0;								 // The last time a Wi-Fi connection was attempted.
unsigned long lastTelemetryPollTime = 0;							 // The last time sensors were polled.
unsigned long lastMqttConnectionTime = 0;							 // The last time a MQTT connection was attempted.
unsigned long lastTelemetryPrintTime = 0;							 // The last time sensor data was printed.
unsigned long lastTelemetryPublishTime = 0;						 // The last time sensor data was printed.
unsigned long lastLedBlinkTime = 0;									 // The last time the LED state was changed (blink).
unsigned long wifiCoolDownInterval = 10000;						 // How long to wait between Wi-Fi connection attempts.
unsigned long mqttCoolDownInterval = 10000;						 // How long to wait between MQTT broker connection attempts.
unsigned long telemetryPollInterval = 10000;						 // How long to wait between sensor polling.
unsigned long telemetryPrintInterval = 10000;					 // How long to wait between sensor printing.
unsigned long telemetryPublishInterval = 20000;					 // How long to wait between sensor printing.
unsigned long ledBlinkInterval = 200;								 // Time between LED state changes (blinks).
char ipAddress[16];														 // A character array to hold the IP address.
char macAddress[18];														 // A character array to hold the MAC address, and append a dash and 3 numbers.
long rssi = 0;																 // A global to hold the Received Signal Strength Indicator.
float tempArray[] = { 21.12, 21.12, 21.12 };						 // An array to hold the 3 most recent temperature values.
float humidityArray[] = { 21.12, 21.12, 21.12 };				 // An array to hold the 3 most recent humidity values.
uint16_t redValue;														 // The APDS9960 red value.
uint16_t greenValue;														 // The APDS9960 green value.
uint16_t blueValue;														 // The APDS9960 blue value.
uint16_t clearValue;														 // The APDS9960 clear/white value.


Adafruit_SHT4x sht40 = Adafruit_SHT4x();
Adafruit_APDS9960 apds;
WiFiClient wifiClient;
PubSubClient mqttClient( wifiClient );


#endif //LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
