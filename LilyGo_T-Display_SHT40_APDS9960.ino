/**
 * @brief This program will read the temperature and humidity from a SHT40 sensor, and light levels from an APDS9960 lux sensor.
 * Values will be read every 15 seconds (configurable), and the current reading will be averaged with the previous two.
 * The devkit is from Lilygo:
 * The STEMMA-QT port is connected to GPIO 43 and 44.
 * The temperature and humidity sensor is from Adafruit: https://www.adafruit.com/products/4885
 * The light sensor is also from Adafruit: https://www.adafruit.com/product/3595
 * ToDo: Add voltage detection using GPIO4
 * ToDo: Use the light sensor color values to set the TFT background color.
 * ToDo: Fix publishing!
 */

#include "LilyGo_T-Display_SHT40_APDS9960.h"

void setup()
{
	delay( 1000 );
	Serial.begin( 115200 );

	if( !Serial )
		delay( 1000 );

	// Set the MAC address variable to its value.
	snprintf( macAddress, 18, "%s", WiFi.macAddress().c_str() );

	pinMode( BACKLIGHT, OUTPUT );

#ifdef OVERRIDE_WIRE
	// Override the default I2C GPIOs.
	Wire.begin( sdaGPIO, sclGPIO );
#else
	// Use default I2C GPIOs.
	Wire.begin();
#endif

	setupSht40();
	setupAPDS();

	wifiConnect();
	configureOTA();

	Serial.println( "Setup has completed." );
} // End of setup() function.

/**
 * @brief setupAPDS() will initialize the SHT30 temperature and humidity sensor.
 */
void setupAPDS()
{
	if( !apds.begin() )
	{
		Serial.println( "\n\n-----------------------------------------------" );
		Serial.println( "Failed to initialize the APDS9960 light sensor!" );
		Serial.println( "Please check your wiring." );
		Serial.println( "-----------------------------------------------\n\n" );
	}
	else
	{
		Serial.println( "APDS9960 initialized!" );
		// Enable color sensing mode
		apds.enableColor( true );
	}
} // End of setupAPDS() function.

/**
 * @brief setupSht40() will initialize the SHT30 temperature and humidity sensor.
 */
void setupSht40()
{
	Serial.println( "Configuring the SHT40" );
	if( !sht40.begin() )
	{
		Serial.println( "Couldn't find the SHT40 on the I2C bus!" );
		Serial.println( "Check the wiring and try again." );
		Serial.println( "\n----------------------------------------" );
		Serial.println( "This program is now in an infinite loop." );
		Serial.println( "----------------------------------------\n" );
		while( 1 )
			delay( 1 );
	}
	Serial.println( "  Found the SHT40" );
	Serial.print( "  Serial number 0x" );
	Serial.println( sht40.readSerial(), HEX );

	// You can have 3 different precisions, higher precision takes longer.
	sht40.setPrecision( SHT4X_HIGH_PRECISION );
	switch( sht40.getPrecision() )
	{
		case SHT4X_HIGH_PRECISION:
			Serial.println( "  SHT40 high precision" );
			break;
		case SHT4X_MED_PRECISION:
			Serial.println( "  SHT40 medium precision" );
			break;
		case SHT4X_LOW_PRECISION:
			Serial.println( "  SHT40 low precision" );
			break;
	}

	// You can have 6 different heater settings.
	// Higher heat and longer times uses more power and reads will take longer too!
	sht40.setHeater( SHT4X_NO_HEATER );
	switch( sht40.getHeater() )
	{
		case SHT4X_NO_HEATER:
			Serial.println( "  No heater" );
			break;
		case SHT4X_HIGH_HEATER_1S:
			Serial.println( "  High heat for 1 second" );
			break;
		case SHT4X_HIGH_HEATER_100MS:
			Serial.println( "  High heat for 0.1 second" );
			break;
		case SHT4X_MED_HEATER_1S:
			Serial.println( "  Medium heat for 1 second" );
			break;
		case SHT4X_MED_HEATER_100MS:
			Serial.println( "  Medium heat for 0.1 second" );
			break;
		case SHT4X_LOW_HEATER_1S:
			Serial.println( "  Low heat for 1 second" );
			break;
		case SHT4X_LOW_HEATER_100MS:
			Serial.println( "  Low heat for 0.1 second" );
			break;
	}
} // End of the setupSht40() function.

/**
 * @brief readColors() will poll the APDS9960 for RGBW values.
 */
void readColors()
{
	// Wait for color data to be ready.
	while( !apds.colorDataReady() )
		delay( 5 );

	// Get color data and print the different channels.
	apds.getColorData( &redValue, &greenValue, &blueValue, &clearValue );
} // End of readColors() function.

/**
 * @brief averageArray() will return the average of values in the passed array.
 */
float averageArray( float valueArray[] )
{
	const unsigned int arraySize = 3;
	float tempValue = 0;
	for( int i = 0; i < arraySize; ++i )
	{
		tempValue += valueArray[i];
	}
	return tempValue / arraySize;
} // End of the averageArray() function.

/**
 * @brief cToF() will convert Celsius to Fahrenheit.
 */
float cToF( float value )
{
	return value * 1.8 + 32;
} // End of the cToF() function.

/**
 * @brief readTelemetry() will manipulate the settingArray[] like a FIFO queue, by popping the head value off, and adding a new value to the tail.
 */
void readTelemetry()
{
	rssi = WiFi.RSSI();
	sensors_event_t humidity;
	sensors_event_t temp;
	sht40.getEvent( &humidity, &temp ); // populate temp and humidity objects with fresh data

	sht40TempCArray[2] = sht40TempCArray[1];
	sht40TempCArray[1] = sht40TempCArray[0];
	sht40TempCArray[0] = temp.temperature;

	sht40HumidityArray[2] = sht40HumidityArray[1];
	sht40HumidityArray[1] = sht40HumidityArray[0];
	sht40HumidityArray[0] = humidity.relative_humidity;

	readColors();

	voltage = ( analogRead( PIN_BAT_VOLT ) * 2 * 3.3 * 1000 ) / 4096;
} // End of readTelemetry() function.


void printADPS()
{
  	Serial.println( "APDS9960 values:" );
	Serial.print( "  Red: " );
	Serial.println( redValue );
	Serial.print( "  Green: " );
	Serial.println( greenValue );
	Serial.print( "  Blue: " );
	Serial.println( blueValue );
	Serial.print( "  Clear: " );
	Serial.println( clearValue );
}


void printTelemetry()
{
	Serial.println();
	printCount++;
	Serial.println( __FILE__ );
	Serial.printf( "Print count: %lu\n", printCount );
	Serial.printf( "Voltage: %d mV\n", voltage );
	Serial.println();

	Serial.println( "Network stats:" );
	Serial.printf( "  MAC address: %s\n", macAddress );
	int wifiStatusCode = WiFi.status();
	char buffer[29];
	lookupWifiCode( wifiStatusCode, buffer );
	if( wifiStatusCode == 3 )
	{
		Serial.printf( "  IP address: %s\n", ipAddress );
		Serial.printf( "  RSSI: %ld\n", rssi );
	}
	Serial.printf( "  wifiConnectCount: %lu\n", wifiConnectCount );
	Serial.printf( "  wifiCoolDownInterval: %lu\n", wifiCoolDownInterval );
	Serial.printf( "  Wi-Fi status text: %s\n", buffer );
	Serial.printf( "  Wi-Fi status code: %d\n", wifiStatusCode );
	Serial.println();

	Serial.println( "MQTT stats:" );
	Serial.printf( "  mqttConnectCount: %lu\n", mqttConnectCount );
	Serial.printf( "  mqttCoolDownInterval: %lu\n", mqttCoolDownInterval );
	Serial.printf( "  Broker: %s:%d\n", mqttBroker, mqttPort );
	lookupMQTTCode( mqttClient.state(), buffer );
	Serial.printf( "  MQTT state: %s\n", buffer );
	Serial.printf( "  Publish count: %lu\n", publishCount );
	Serial.printf( "  Callback count: %lu\n", callbackCount );
	Serial.println();

	Serial.println( "Environmental stats:" );
	Serial.printf( "  SHT40 temp: %.3f C\n", sht40TempCArray[0] );
	Serial.printf( "  Average: %.3f C\n", averageArray( sht40TempCArray ) );
	Serial.printf( "  Average: %.3f F\n", cToF( averageArray( sht40TempCArray ) ) );
	Serial.printf( "  SHT40 humidity: %.3f %%\n", sht40HumidityArray[0] );
	Serial.printf( "  Average: %.3f %%\n", averageArray( sht40HumidityArray ) );
	Serial.println();

  printADPS();

	Serial.println();
} // End of printTelemetry() function.

/**
 * @brief toggleLED() will change the state of the LED.
 * This function does not manage any timings.
 */
void toggleLED()
{
	if( digitalRead( BACKLIGHT ) != 1 )
		digitalWrite( BACKLIGHT, 1 );
	else
		digitalWrite( BACKLIGHT, 0 );
} // End of toggleLED() function.

void loop()
{
	// Reconnect Wi-Fi if needed, reconnect MQTT if needed, and process MQTT and OTA requests.
	if( WiFi.status() != WL_CONNECTED )
		wifiConnect();
	else if( !mqttClient.connected() )
		mqttConnect();
	else
	{
		mqttClient.loop();
		ArduinoOTA.handle();
	}

	unsigned long currentTime = millis();
	// Poll the first time.  Avoid subtraction overflow.  Poll every interval.
	if( lastTelemetryPollTime == 0 || ( ( currentTime > telemetryPollInterval ) && ( currentTime - telemetryPollInterval ) > lastTelemetryPollTime ) )
	{
		readTelemetry();
		lastTelemetryPollTime = millis();
	}

	currentTime = millis();
	if( lastTelemetryPrintTime == 0 || ( ( currentTime > telemetryPrintInterval ) && ( currentTime - telemetryPrintInterval ) > lastTelemetryPrintTime ) )
	{
		printTelemetry();
    lastAdpsPrintTime = currentTime;
		lastTelemetryPrintTime = currentTime;
	}

	currentTime = millis();
	if( mqttClient.connected() && ( lastTelemetryPublishTime == 0 || ( ( currentTime > telemetryPublishInterval ) && ( currentTime - telemetryPublishInterval ) > lastTelemetryPublishTime ) ) )
	{
		publishTelemetry();
		lastTelemetryPublishTime = millis();
	}

	currentTime = millis();
	// Process the first time.  Avoid subtraction overflow.  Process every interval.
	if( lastLedBlinkTime == 0 || ( ( currentTime > ledBlinkInterval ) && ( currentTime - ledBlinkInterval ) > lastLedBlinkTime ) )
	{
		lastLedBlinkTime = millis();

		// If Wi-Fi is connected, but MQTT is not, blink the LED.
		if( WiFi.status() == WL_CONNECTED )
		{
			if( mqttClient.state() != 0 )
				toggleLED(); // Blink the backlight to show that Wi-Fi is connected, but MQTT is not.
			else
				digitalWrite( BACKLIGHT, 1 ); // Turn the backlight on to show both Wi-Fi and MQTT are connected.
		}
		else
			digitalWrite( BACKLIGHT, 0 ); // Turn the backlight off to show that Wi-Fi is not connected.
	}

	currentTime = millis();
	// Process the first time.  Avoid subtraction overflow.  Process every interval.
	if( ( currentTime - 2000 ) > lastAdpsPrintTime )
	{
    readTelemetry();
    printADPS();
    lastLedBlinkTime = currentTime;
	}
} // End of loop() function.
