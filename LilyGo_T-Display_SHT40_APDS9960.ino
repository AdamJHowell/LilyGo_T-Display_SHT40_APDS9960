/**
 * @brief This program will read the temperature and humidity from a SHT40 sensor, and light levels from an APDS9960 lux sensor.
 * Values will be read every 15 seconds (configurable), and the current reading will be averaged with the previous two.
 * The devkit is from Lilygo:
 * The STEMMA-QT port is connected to GPIO 43 and 44.
 * The temperature and humidity sensor is from Adafruit: https://www.adafruit.com/products/4885
 * The light sensor is also from Adafruit: https://www.adafruit.com/product/3595
 * ToDo: Add voltage detection using GPIO4
 */

#include "LilyGo_T-Display_SHT40_APDS9960.h"


void setup()
{
	delay( 1000 );
	Serial.begin( 115200 );

	if( !Serial )
		delay( 1000 );

	pinMode( BACKLIGHT, OUTPUT );

#ifdef OVERRIDE_WIRE
	// Override the default I2C GPIOs.
	Wire.begin( sdaGPIO, sclGPIO );
#else
	// Use default I2C GPIOs.
	 Wire.begin();
#endif

	Serial.println( "Adafruit SHT4x test" );
	if( !sht40.begin())
	{
		Serial.println( "Couldn't find SHT4x" );
		while( 1 )
			delay( 1 );
	}
	Serial.println( "Found SHT4x sensor" );
	Serial.print( "Serial number 0x" );
	Serial.println( sht40.readSerial(), HEX );

	// You can have 3 different precisions, higher precision takes longer
	sht40.setPrecision( SHT4X_HIGH_PRECISION );
	switch( sht40.getPrecision())
	{
		case SHT4X_HIGH_PRECISION:
			Serial.println( "High precision" );
			break;
		case SHT4X_MED_PRECISION:
			Serial.println( "Med precision" );
			break;
		case SHT4X_LOW_PRECISION:
			Serial.println( "Low precision" );
			break;
	}

	// You can have 6 different heater settings
	// higher heat and longer times uses more power
	// and reads will take longer too!
	sht40.setHeater( SHT4X_NO_HEATER );
	switch( sht40.getHeater())
	{
		case SHT4X_NO_HEATER:
			Serial.println( "No heater" );
			break;
		case SHT4X_HIGH_HEATER_1S:
			Serial.println( "High heat for 1 second" );
			break;
		case SHT4X_HIGH_HEATER_100MS:
			Serial.println( "High heat for 0.1 second" );
			break;
		case SHT4X_MED_HEATER_1S:
			Serial.println( "Medium heat for 1 second" );
			break;
		case SHT4X_MED_HEATER_100MS:
			Serial.println( "Medium heat for 0.1 second" );
			break;
		case SHT4X_LOW_HEATER_1S:
			Serial.println( "Low heat for 1 second" );
			break;
		case SHT4X_LOW_HEATER_100MS:
			Serial.println( "Low heat for 0.1 second" );
			break;
	}
	if( !apds.begin())
		Serial.println( "Failed to initialize device! Please check your wiring." );
	else Serial.println( "Device initialized!" );

	// Enable color sensing mode
	apds.enableColor( true );

	Serial.println( "Setup has completed." );
} // End of setup() function.


/**
 * @brief readTelemetry() will manipulate the settingArray[] like a FIFO queue, by popping the head value off, and adding a new value to the tail.
 */
void readTelemetry()
{
	sensors_event_t humidity;
	sensors_event_t temp;
	sht40.getEvent( &humidity, &temp );// populate temp and humidity objects with fresh data

	tempArray[0] = tempArray[1];
	tempArray[1] = tempArray[2];
	tempArray[2] = temp.temperature;

	humidityArray[0] = humidityArray[1];
	humidityArray[1] = humidityArray[2];
	humidityArray[2] = humidity.relative_humidity;
} // End of readTelemetry() function.


void printTelemetry()
{
	Serial.printf( "Current temp: %.3f degrees C\n", tempArray[2] );
	float tempC = ( tempArray[0] + tempArray[1] + tempArray[2] ) / 3.0;
	Serial.printf( "Average: %.3f degrees C\n", tempC );
	float tempF = ( tempC * 1.8 ) + 32;
	Serial.printf( "Average: %.3f degrees F\n", tempF );

	Serial.printf( "Current humidity: %.3f %% rH\n", humidityArray[2] );
	float humidity = ( humidityArray[0] + humidityArray[1] + humidityArray[2] ) / 3.0;
	Serial.printf( "Average: %.3f %% rH\n", humidity );
} // End of printTelemetry() function.


void readColors()
{
	// Variables to store the color data.
	uint16_t r, g, b, c;

	// Wait for color data to be ready.
	while( !apds.colorDataReady())
		delay( 5 );

	// Get color data and print the different channels.
	apds.getColorData( &r, &g, &b, &c );
	Serial.print( "red: " );
	Serial.print( r );

	Serial.print( " green: " );
	Serial.print( g );

	Serial.print( " blue: " );
	Serial.print( b );

	Serial.print( " clear: " );
	Serial.println( c );
	Serial.println();
} // End of readColors() function.


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
	unsigned long time = millis();
	// Poll the first time.  Avoid subtraction overflow.  Poll every interval.
	if( lastTelemetryReadTime == 0 || (( time > telemetryPollInterval ) && ( time - telemetryPollInterval ) > lastTelemetryReadTime ))
	{
		readTelemetry();
		lastTelemetryReadTime = millis();

		readColors();
		toggleLED();
	}
	time = millis();
	if( lastTelemetryPrintTime == 0 || (( time > telemetryPrintInterval ) && ( time - telemetryPrintInterval ) > lastTelemetryPrintTime ))
	{
		printTelemetry();
		lastTelemetryPrintTime = millis();
	}
} // End of loop() function.
