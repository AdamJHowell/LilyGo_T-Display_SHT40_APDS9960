//
// Created by Adam Howell on 2022-12-06.
//

#ifndef LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
#define LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H

/**
 * Function declarations.
 */
// NetworkFunctions.ino
void onReceiveCallback( char *topic, byte *payload, unsigned int length );
void configureOTA();
int checkForSSID( const char *ssidName );
void wifiMultiConnect();
int mqttMultiConnect( int maxAttempts );
void publishStats();
void publishTelemetry();
void lookupWifiCode( int code, char *buffer );
void lookupMQTTCode( int code, char *buffer );
// LillyGo_T-Display_SHT40_ADPS9960.ino
void setup();
void readTelemetry();
void printTelemetry();
void readColors();
void toggleLED();
void loop();


#include "privateInfo.h"
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
const char *HOST_NAME = "LegoEsp32SunTracker";				  // The hostname used for OTA access.
const char *SKETCH_NAME = "LegoEsp32SunTracker.ino";		  // The name used when publishing stats.
const char *NOTES = "The ESP32 sun tracker";					  // The hostname used for OTA access.
const char *MQTT_STATS_TOPIC = "sunTracker/stats";			  // The topic this device will publish to upon connection to the broker.
const char *MQTT_COMMAND_TOPIC = "sunTracker/commands";	  // The command topic this device will subscribe to.
const char *sketchTopic = "sunTracker/sketch";				  // The topic used to publish the sketch name (__FILE__).
const char *macTopic = "sunTracker/mac";						  // The topic used to publish the MAC address.
const char *ipTopic = "sunTracker/ip";							  // The topic used to publish the IP address.
const char *rssiTopic = "sunTracker/rssi";					  // The topic used to publish the Wi-Fi Received Signal Strength Indicator.
const char *publishCountTopic = "sunTracker/publishCount"; // The topic used to publish the loop count.
const char *notesTopic = "sunTracker/notes";					  // The topic used to publish notes relevant to this project.
const unsigned long JSON_DOC_SIZE = 512;						  // The ArduinoJson document size, and size of some buffers.
/**
 * Global variables.
 */
char ipAddress[16];									// A character array to hold the IP address.
char macAddress[18];									// A character array to hold the MAC address, and append a dash and 3 numbers.
long rssi = 0;											// A global to hold the Received Signal Strength Indicator.
unsigned int networkIndex = 2112;				// An unsigned integer to hold the correct index for the network arrays: wifiSsidArray[], wifiPassArray[], mqttBrokerArray[], and mqttPortArray[].
unsigned int wifiConnectionTimeout = 10000;	// Set the Wi-Fi connection timeout to 10 seconds.
unsigned int mqttReconnectInterval = 3000;	// Set the minimum time between sequential MQTT broker connection attempts to 3 seconds.
unsigned int mqttReconnectCooldown = 20000;	// Set the minimum time between calls to mqttMultiConnect() to 20 seconds.
unsigned int telemetryProcessInterval = 200; // How long to wait between sensor processing.
unsigned int publishInterval = 60000;			// How long to wait between MQTT publishes.
unsigned int callbackCount = 0;					// The number of times a callback was received.
unsigned long publishCount = 0;					// A counter of how many times the stats have been published.
unsigned long lastTelemetryPollTime = 0;		// The last time sensors were polled.
unsigned long lastTelemetryProcessTime = 0;	// The last time sensor data was acted on.
unsigned long lastPublishTime = 0;				// The last time a MQTT publish was performed.
unsigned long lastMqttConnectionTime = 0;		// The last time a MQTT connection was attempted.


Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_APDS9960 apds;


#endif  //LILYGO_T_DISPLAY_SHT40_APDS9960_LILYGO_T_DISPLAY_SHT40_APDS9960_H
